#include "Application.h"
#include "logger.h"

Application::Application(std::unique_ptr<ManifestParser> parser) : parser(std::move(parser)) {
    LOG_INFO("Application initialized.");
}

bool Application::run(const std::string &manifest_path) {
    if (!parser) {
        LOG_ERROR("Manifest parser is not initialized.");
        return false;
    }

    LOG_INFO("Parsing manifest: {}", manifest_path);
    if (!parser->parse(manifest_path)) {
        LOG_ERROR("Failed to parse manifest.");
        return false;
    }

    const auto &keys = parser->getKeys();
    if (keys.empty()) {
        LOG_ERROR("No keys found in the manifest.");
        return false;
    }

    LOG_INFO("Acquiring license for key: {}", keys.front());
    if (!license_helper.acquireLicense(keys.front())) {
        LOG_ERROR("Failed to acquire license.");
        return false;
    }

    const std::string &media_url = parser->getMediaUrl();
    if (media_url.empty()) {
        LOG_ERROR("No media URL found in the manifest.");
        return false;
    }

    LOG_INFO("Initializing pipeline for media URL: {}", media_url);
    if (!pipeline_manager.initialize(media_url)) {
        LOG_ERROR("Failed to initialize the pipeline.");
        return false;
    }

    LOG_INFO("Starting playback.");
    if (!pipeline_manager.play()) {
        LOG_ERROR("Failed to start playback.");
        return false;
    }

    LOG_INFO("Pipeline is playing. Press Ctrl+C to stop...");
    GMainLoop *main_loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(main_loop);
    g_main_loop_unref(main_loop);

    LOG_INFO("Stopping pipeline.");
    pipeline_manager.stop();
    return true;
}
