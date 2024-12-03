#ifndef LOGGER_H
#define LOGGER_H

#include <gst/gst.h>

extern GstDebugCategory *drm_probe_debug;

#define LOG_DEBUG(fmt, ...) GST_CAT_LEVEL_LOG(drm_probe_debug, GST_LEVEL_DEBUG, NULL, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) GST_CAT_LEVEL_LOG(drm_probe_debug, GST_LEVEL_INFO, NULL, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) GST_CAT_LEVEL_LOG(drm_probe_debug, GST_LEVEL_WARNING, NULL, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) GST_CAT_LEVEL_LOG(drm_probe_debug, GST_LEVEL_ERROR, NULL, fmt, ##__VA_ARGS__)

void initialize_logging();

#endif // LOGGER_H
