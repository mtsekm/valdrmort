#include "logger.h"

GstDebugCategory *drm_probe_debug = nullptr;

void initialize_logging() {
    if (!drm_probe_debug) {
        GST_DEBUG_CATEGORY_INIT(drm_probe_debug, "valdrmort", 0, "DRM Probe Plugin");
    }
}