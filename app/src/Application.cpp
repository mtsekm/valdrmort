#include "Application.h"
#include "Logger.h"

Application::Application(std::unique_ptr<ManifestParser> parser) : parser(std::move(parser)) {
    logInfo("Application initialized.");
}

bool Application::run(const std::string &manifest_path) {
    if (!parser) {
        logError("Manifest parser is not initialized.");
        return false;
    }

    logInfo("Parsing manifest: " + manifest_path);
    if (!parser->parse(manifest_path)) {
        logError("Failed to parse manifest.");
        return false;
    }

    const auto &keys = parser->getDRMKeys();
    if (keys.empty()) {
        logError("No keys found in the manifest.");
        return false;
    }

    logInfo("Acquiring license for key: " + keys.front());
    if (!license_helper.acquireLicense(keys.front())) {
        logError("Failed to acquire license.");
        return false;
    }

    const std::string &media_url = parser->getStreamURI();
    if (media_url.empty()) {
        logError("No media URL found in the manifest.");
        return false;
    }

    logInfo("Initializing pipeline for media URL: " + media_url);
    if (!pipeline_manager.initialize(media_url)) {
        logError("Failed to initialize the pipeline.");
        return false;
    }

    logInfo("Starting playback.");
    if (!pipeline_manager.play()) {
        logError("Failed to start playback.");
        return false;
    }

    logInfo("Pipeline is playing. Press Ctrl+C to stop...");
    GMainLoop *main_loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(main_loop);
    g_main_loop_unref(main_loop);

    logInfo("Stopping pipeline.");
    pipeline_manager.stop();
    return true;
}
