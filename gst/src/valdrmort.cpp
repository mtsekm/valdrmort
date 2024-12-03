#include "valdrmort_element.h"
#include "logger.h"

#include <gst/gst.h>

#include "config.h"

static gboolean valdrmort_init(GstPlugin *plugin) {
    initialize_logging();

    LOG_INFO("Initializing valdrmort plugin");

    if (!gst_element_register(plugin, "valdrmort", GST_RANK_PRIMARY, VALDRMORT_TYPE_ELEMENT)) {
        LOG_ERROR("Failed to register valdrmort element.");
        return FALSE;
    }

    LOG_INFO("valdrmort plugin initialized successfully.");
    return TRUE;
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, valdrmort, "valdrmort GStreamer Plugin", valdrmort_init, VERSION,
                  "MIT", PACKAGE_NAME, "https://github.com/mtsekm/valdrmort")
