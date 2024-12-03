#include "Application.h"
#include "M3U8ManifestParser.h"
#include "MPDManifestParser.h"
#include "logger.h"
#include <filesystem>
#include <memory>

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Usage: %s <manifest path>", argv[0]);
        return 1;
    }

    std::string manifest_path = argv[1];
    std::filesystem::path file_path(manifest_path);

    if (!std::filesystem::exists(file_path)) {
        LOG_ERROR("Manifest file does not exist: %s", manifest_path.c_str());
        return 1;
    }

    // Determine manifest type from file extension
    std::string extension = file_path.extension().string();
    std::unique_ptr<ManifestParser> parser;

    if (extension == ".mpd") {
        LOG_INFO("Detected MPD manifest file.");
        parser = std::make_unique<MPDManifestParser>();
    } else if (extension == ".m3u8") {
        LOG_INFO("Detected M3U8 manifest file.");
        parser = std::make_unique<M3U8ManifestParser>();
    } else {
        LOG_ERROR("Unsupported manifest type: %s", extension.c_str());
        return 1;
    }

    Application app(std::move(parser));
    if (!app.run(manifest_path)) {
        LOG_ERROR("Test application failed.");
        return 1;
    }

    LOG_INFO("Application completed successfully.");
    return 0;
}
