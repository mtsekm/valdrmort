#ifndef PIPELINE_MANAGER_H
#define PIPELINE_MANAGER_H

#include <gst/gst.h>

#include <string>

class PipelineManager {
public:
    PipelineManager();
    ~PipelineManager();

    // Initialize the GStreamer pipeline
    bool initialize(const std::string &manifestUrl);

    // Start playing the media
    bool play();

    // Pause the media playback
    bool pause();

    // Stop and clean up the pipeline
    void stop();

    // Get the current playback state
    GstState getState() const;

private:
    // GStreamer pipeline and related elements
    GstElement *pipeline;
    GstElement *source;
    GstElement *dashdemuxer;
    GstElement *qtdemuxer;
    GstElement *decryptor;
    GstElement *decoder;
    GstElement *sink;

    // Utility function to handle dynamic pad linking
    static void onPadAdded(GstElement *element, GstPad *pad, gpointer data);

    // Helper for GStreamer bus messages
    static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer user_data);

    // Error handling
    void handleError(GstMessage *message);
};

#endif // PIPELINE_MANAGER_H
