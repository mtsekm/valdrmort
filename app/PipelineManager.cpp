#include "PipelineManager.h"
#include "logger.h"

PipelineManager::PipelineManager()
    : pipeline(nullptr), source(nullptr), dashdemuxer(nullptr), qtdemuxer(nullptr), decryptor(nullptr),
      decoder(nullptr), sink(nullptr) {
    gst_init(nullptr, nullptr);
    LOG_INFO("PipelineManager initialized.");
}

PipelineManager::~PipelineManager() {
    stop();
    LOG_INFO("PipelineManager destroyed.");
}

bool PipelineManager::initialize(const std::string &manifestUrl) {
    LOG_INFO("Initializing pipeline with manifest URL: %s", manifestUrl.c_str());

    // Create GStreamer elements
    pipeline = gst_pipeline_new("valdrmort-pipeline");
    source = gst_element_factory_make("souphttpsrc", "http-source");
    dashdemuxer = gst_element_factory_make("dashdemux", "dash-demuxer");
    qtdemuxer = gst_element_factory_make("qtdemux", "qt-demuxer");
    decryptor = gst_element_factory_make("valdrmort", "drm-decryptor");
    decoder = gst_element_factory_make("decodebin", "decoder");
    sink = gst_element_factory_make("autovideosink", "video-sink");

    if (!pipeline || !source || !dashdemuxer || !qtdemuxer || !decryptor || !decoder || !sink) {
        LOG_ERROR("Failed to create one or more GStreamer elements.");
        return false;
    }

    // Set properties
    g_object_set(G_OBJECT(source), "location", manifestUrl.c_str(), nullptr);

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, dashdemuxer, qtdemuxer, decryptor, decoder, sink, nullptr);

    // Link static elements
    if (!gst_element_link(source, dashdemuxer)) {
        LOG_ERROR("Failed to link source to dashdemuxer.");
        return false;
    }
    if (!gst_element_link(dashdemuxer, qtdemuxer)) {
        LOG_ERROR("Failed to link dashdemuxer to qtdemuxer.");
        return false;
    }
    if (!gst_element_link(qtdemuxer, decryptor)) {
        LOG_ERROR("Failed to link qtdemuxer to decryptor.");
        return false;
    }
    if (!gst_element_link(decryptor, decoder)) {
        LOG_ERROR("Failed to link decryptor to decoder.");
        return false;
    }

    // Connect dynamic pad handler for decodebin
    g_signal_connect(decoder, "pad-added", G_CALLBACK(onPadAdded), sink);

    return true;
}

bool PipelineManager::play() {
    if (!pipeline) {
        LOG_ERROR("Pipeline not initialized.");
        return false;
    }

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LOG_ERROR("Failed to set pipeline state to PLAYING.");
        return false;
    }

    LOG_INFO("Pipeline started playing.");
    return true;
}

bool PipelineManager::pause() {
    if (!pipeline) {
        LOG_ERROR("Pipeline not initialized.");
        return false;
    }

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LOG_ERROR("Failed to set pipeline state to PAUSED.");
        return false;
    }

    LOG_INFO("Pipeline paused.");
    return true;
}

void PipelineManager::stop() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
        LOG_INFO("Pipeline stopped and resources released.");
    }
}

GstState PipelineManager::getState() const {
    if (!pipeline) {
        LOG_WARN("Pipeline not initialized. Returning NULL state.");
        return GST_STATE_NULL;
    }

    GstState currentState;
    gst_element_get_state(pipeline, &currentState, nullptr, GST_CLOCK_TIME_NONE);
    return currentState;
}

void PipelineManager::onPadAdded(GstElement *element, GstPad *pad, gpointer data) {
    GstElement *sink = GST_ELEMENT(data);

    GstPad *sinkPad = gst_element_get_static_pad(sink, "sink");
    if (gst_pad_is_linked(sinkPad)) {
        LOG_DEBUG("Pad is already linked.");
        gst_object_unref(sinkPad);
        return;
    }

    GstCaps *caps = gst_pad_query_caps(pad, nullptr);
    const gchar *name = gst_structure_get_name(gst_caps_get_structure(caps, 0));
    LOG_DEBUG("Dynamic pad added: %s", name);

    if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
        LOG_ERROR("Failed to link dynamic pad.");
    } else {
        LOG_INFO("Dynamic pad linked successfully.");
    }

    gst_caps_unref(caps);
    gst_object_unref(sinkPad);
}

gboolean PipelineManager::busCallback(GstBus *bus, GstMessage *message, gpointer user_data) {
    auto *manager = static_cast<PipelineManager *>(user_data);

    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR:
        manager->handleError(message);
        break;
    case GST_MESSAGE_EOS:
        LOG_INFO("End of stream reached.");
        break;
    default:
        break;
    }
    return TRUE;
}

void PipelineManager::handleError(GstMessage *message) {
    GError *err = nullptr;
    gchar *debugInfo = nullptr;
    gst_message_parse_error(message, &err, &debugInfo);

    LOG_ERROR("Error from element %s: %s", GST_OBJECT_NAME(message->src), err->message);
    if (debugInfo) {
        LOG_DEBUG("Debug info: %s", debugInfo);
    }

    g_clear_error(&err);
    g_free(debugInfo);
}
