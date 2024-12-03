#include "valdrmort_element.h"
#include "DRMBackendFactory.h"
#include "config.h"
#include "logger.h"

#include <gst/base/gstbasetransform.h>
#include <gst/gst.h>
#include <gst/gstelement.h>
#include <gst/gstprotection.h>

#include <iomanip>
#include <sstream>

// Pad templates
static GstStaticPadTemplate sink_pad_template = GST_STATIC_PAD_TEMPLATE(
    "sink", GST_PAD_SINK, GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/x-cenc, protection-system=(string)" CLEARKEY_PROTECTION_ID "; "
                    "application/x-cenc, protection-system=(string)" WIDEVINE_PROTECTION_ID "; "
                    "application/x-cenc, protection-system=(string)" PLAYREADY_PROTECTION_ID "; "
                    "application/x-cenc, protection-system=(string)" FAIRPLAY_PROTECTION_ID));

static GstStaticPadTemplate src_pad_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);

// static GstStaticPadTemplate src_pad_template =
//     GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("video/x-raw"));

static const gchar *valdrmort_decrypt_protection_ids[] = {CLEARKEY_PROTECTION_ID, WIDEVINE_PROTECTION_ID,
                                                          PLAYREADY_PROTECTION_ID, FAIRPLAY_PROTECTION_ID, NULL};

// GObject type definition
G_DEFINE_TYPE(valdrmortElement, valdrmort_element, GST_TYPE_BASE_TRANSFORM);

// Function prototypes
static gboolean valdrmort_start(GstBaseTransform *trans);
static gboolean valdrmort_stop(GstBaseTransform *trans);
static gboolean valdrmort_append_if_not_duplicate(GstCaps *dest, GstStructure *new_struct);
static GstCaps *valdrmort_transform_caps(GstBaseTransform *base, GstPadDirection direction, GstCaps *caps,
                                         GstCaps *filter);
static GstFlowReturn valdrmort_transform_ip(GstBaseTransform *trans, GstBuffer *buf);
static gboolean valdrmort_sink_event_handler(GstBaseTransform *trans, GstEvent *event);
static void valdrmort_finalize(GObject *object);

// Initialization
static void valdrmort_element_class_init(valdrmortElementClass *klass) {
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS(klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&sink_pad_template));
    gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&src_pad_template));
    gst_element_class_set_static_metadata(element_class, "valdrmort element", GST_ELEMENT_FACTORY_KLASS_DECRYPTOR,
                                          "Decrypt DRM-protected media", "Tarik Sekmen <tarik.sekmen@sky.uk>");

    base_transform_class->start = GST_DEBUG_FUNCPTR(valdrmort_start);
    base_transform_class->stop = GST_DEBUG_FUNCPTR(valdrmort_stop);
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR(valdrmort_transform_ip);
    base_transform_class->sink_event = GST_DEBUG_FUNCPTR(valdrmort_sink_event_handler);
    base_transform_class->transform_caps = GST_DEBUG_FUNCPTR(valdrmort_transform_caps);

    gobject_class->finalize = valdrmort_finalize;
}

static void valdrmort_element_init(valdrmortElement *self) {
    LOG_INFO("valdrmort element init");
    self->drm_backend = nullptr;
    self->initialized = FALSE;
}

// Start function
static gboolean valdrmort_start(GstBaseTransform *trans) {
    LOG_INFO("Starting valdrmort Element");

    GstPad *sinkpad = GST_BASE_TRANSFORM_SINK_PAD(trans);
    GstCaps *caps = gst_pad_get_current_caps(sinkpad);
    if (caps) {
        LOG_INFO("Caps at start: %s", gst_caps_to_string(caps));
        gst_caps_unref(caps);
    } else {
        LOG_WARN("No caps negotiated at start");
    }

    return TRUE;
}

// Stop function
static gboolean valdrmort_stop(GstBaseTransform *trans) {
    auto *self = VALDRMORT_ELEMENT(trans);
    LOG_INFO("Stopping valdrmort Element");
    self->drm_backend.reset();
    return TRUE;
}

/* filter out the audio and video related fields from the up-stream caps,
   because they are not relevant to the input caps of this element and
   can cause caps negotiation failures with adaptive bitrate streams */
static void
gst_cenc_remove_codec_fields (GstStructure *gs)
{
  gint j, n_fields = gst_structure_n_fields (gs);
  for(j=n_fields-1; j>=0; --j){
    const gchar *field_name;

    field_name = gst_structure_nth_field_name (gs, j);
    GST_TRACE ("Check field \"%s\" for removal", field_name);

    if( g_strcmp0 (field_name, "base-profile")==0 ||
        g_strcmp0 (field_name, "codec_data")==0 ||
        g_strcmp0 (field_name, "height")==0 ||
        g_strcmp0 (field_name, "framerate")==0 ||
        g_strcmp0 (field_name, "level")==0 ||
        g_strcmp0 (field_name, "pixel-aspect-ratio")==0 ||
        g_strcmp0 (field_name, "profile")==0 ||
        g_strcmp0 (field_name, "rate")==0 ||
        g_strcmp0 (field_name, "width")==0 ){
      gst_structure_remove_field (gs, field_name);
      GST_TRACE ("Removing field %s", field_name);
    }
  }
}

/*
  Append new_structure to dest, but only if it does not already exist in res.
  This function takes ownership of new_structure.
*/
static gboolean valdrmort_append_if_not_duplicate(GstCaps *dest, GstStructure *new_struct) {
    gboolean duplicate = FALSE;
    guint j;

    for (j = 0; !duplicate && j < gst_caps_get_size(dest); ++j) {
        GstStructure *s = gst_caps_get_structure(dest, j);
        if (gst_structure_is_equal(s, new_struct)) {
            duplicate = TRUE;
        }
    }
    if (!duplicate) {
        gst_caps_append_structure(dest, new_struct);
    } else {
        gst_structure_free(new_struct);
    }
    return duplicate;
}

// Caps transformation
static GstCaps *valdrmort_transform_caps(GstBaseTransform *base, GstPadDirection direction, GstCaps *caps,
                                         GstCaps *filter) {
    GstCaps *res = NULL;
    guint i;
    gint j;

    g_return_val_if_fail(direction != GST_PAD_UNKNOWN, NULL);

    LOG_DEBUG("direction: %s   caps: %" GST_PTR_FORMAT "   filter:"
              " %" GST_PTR_FORMAT,
              (direction == GST_PAD_SRC) ? "Src" : "Sink", caps, filter);

    if (direction == GST_PAD_SRC && gst_caps_is_any(caps)) {
        res = gst_pad_get_pad_template_caps(GST_BASE_TRANSFORM_SINK_PAD(base));
        goto filter;
    }

    res = gst_caps_new_empty();

    for (i = 0; i < gst_caps_get_size(caps); ++i) {
        GstStructure *in = gst_caps_get_structure(caps, i);
        GstStructure *out = NULL;

        if (direction == GST_PAD_SINK) {
            gint n_fields;

            if (!gst_structure_has_field(in, "original-media-type"))
                continue;

            out = gst_structure_copy(in);
            n_fields = gst_structure_n_fields(in);

            gst_structure_set_name(out, gst_structure_get_string(out, "original-media-type"));

            /* filter out the DRM related fields from the down-stream caps */
            for (j = n_fields - 1; j >= 0; --j) {
                const gchar *field_name;

                field_name = gst_structure_nth_field_name(in, j);

                if (g_str_has_prefix(field_name, "protection-system") ||
                    g_str_has_prefix(field_name, "original-media-type")) {
                    gst_structure_remove_field(out, field_name);
                }
            }
            valdrmort_append_if_not_duplicate(res, out);
        } else { /* GST_PAD_SRC */
            GstStructure *tmp = NULL;
            guint p;
            tmp = gst_structure_copy(in);
            gst_cenc_remove_codec_fields(tmp);
            for (p = 0; valdrmort_decrypt_protection_ids[p]; ++p) {
                /* filter out the audio/video related fields from the down-stream
                   caps, because they are not relevant to the input caps of this
                   element and they can cause caps negotiation failures with
                   adaptive bitrate streams */
                out = gst_structure_copy(tmp);
                gst_structure_set(out, "protection-system", G_TYPE_STRING, valdrmort_decrypt_protection_ids[p],
                                  "original-media-type", G_TYPE_STRING, gst_structure_get_name(in), NULL);
                gst_structure_set_name(out, "application/x-cenc");
                valdrmort_append_if_not_duplicate(res, out);
            }
            gst_structure_free(tmp);
        }
    }
    if (direction == GST_PAD_SINK && gst_caps_get_size(res) == 0) {
        gst_caps_unref(res);
        res = gst_caps_new_any();
    }

filter:
    if (filter) {
        GstCaps *intersection;

        LOG_DEBUG("Using filter caps %" GST_PTR_FORMAT, filter);
        intersection = gst_caps_intersect_full(res, filter, GST_CAPS_INTERSECT_FIRST);
        gst_caps_unref(res);
        res = intersection;
    }

    LOG_DEBUG(" returning %" GST_PTR_FORMAT, res);
    return res;
}

// Sink event handler
static gboolean valdrmort_sink_event_handler(GstBaseTransform *trans, GstEvent *event) {
    auto *self = VALDRMORT_ELEMENT(trans);

    switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_PROTECTION: {
        const gchar *system_id = nullptr;
        GstBuffer *data = nullptr;
        const gchar *origin = nullptr;

        gst_event_parse_protection(event, &system_id, &data, &origin);
        LOG_INFO("Received protection event: system_id=%s, location=%s", system_id, origin);

        if (data) {
            GstMapInfo map_info;
            if (gst_buffer_map(data, &map_info, GST_MAP_READ)) {
                LOG_INFO("Protection data buffer size: %zu bytes", map_info.size);

                std::ostringstream hex_stream;
                hex_stream << "Protection data (hex): ";
                for (size_t i = 0; i < map_info.size; ++i) {
                    hex_stream << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
                               << static_cast<int>(map_info.data[i]);
                    if (i < map_info.size - 1) {
                        hex_stream << " ";
                    }
                }
                LOG_DEBUG("%s", hex_stream.str().c_str());
                gst_buffer_unmap(data, &map_info);
            } else {
                LOG_WARN("Failed to map protection data buffer.");
            }
        }

        try {
            self->drm_backend = DRMBackendFactory::create(system_id);
            if (!self->drm_backend->initialize()) {
                LOG_ERROR("Failed to initialize DRM backend for system ID: %s", system_id);
                return FALSE;
            }
            self->initialized = TRUE;
            LOG_INFO("DRM backend initialized for system ID: %s", system_id);
        } catch (const std::exception &e) {
            LOG_ERROR("Error initializing DRM backend: %s", e.what());
            return FALSE;
        }
        break;
    }
    default:
        break;
    }

    return GST_BASE_TRANSFORM_CLASS(valdrmort_element_parent_class)->sink_event(trans, event);
}

// Transform in-place
static GstFlowReturn valdrmort_transform_ip(GstBaseTransform *trans, GstBuffer *buf) {
    auto *self = VALDRMORT_ELEMENT(trans);

    if (!self->initialized || !self->drm_backend) {
        LOG_ERROR("valdrmort Element not initialized");
        return GST_FLOW_ERROR;
    }

    GstMapInfo map_info;
    if (!gst_buffer_map(buf, &map_info, GST_MAP_READWRITE)) {
        LOG_ERROR("Failed to map input buffer");
        return GST_FLOW_ERROR;
    }

    // Decrypt buffer
    std::vector<uint8_t> encrypted_data(map_info.data, map_info.data + map_info.size);
    std::vector<uint8_t> decrypted_data;

    if (!self->drm_backend->decrypt(encrypted_data, decrypted_data)) {
        LOG_ERROR("Decryption failed");
        gst_buffer_unmap(buf, &map_info);
        return GST_FLOW_ERROR;
    }

    gst_buffer_unmap(buf, &map_info);

    // Replace buffer contents with decrypted data
    gst_buffer_fill(buf, 0, decrypted_data.data(), decrypted_data.size());

    LOG_DEBUG("Buffer decrypted successfully");
    return GST_FLOW_OK;
}

// Finalize function
static void valdrmort_finalize(GObject *object) {
    auto *self = VALDRMORT_ELEMENT(object);
    LOG_INFO("Finalizing valdrmort Element");
    self->drm_backend.reset();
    G_OBJECT_CLASS(valdrmort_element_parent_class)->finalize(object);
}
