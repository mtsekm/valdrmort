#include "Application.h"
#include "M3U8ManifestParser.h"
#include "MPDManifestParser.h"
#include "Logger.h"
#include <filesystem>
#include <memory>

int main(int argc, char **argv) {
    if (argc < 2) {
        logError("Usage: valdrmort <manifest path>");
        return 1;
    }

    std::string manifest_path = argv[1];
    std::filesystem::path file_path(manifest_path);

    if (!std::filesystem::exists(file_path)) {
        logError("Manifest file does not exist: " + manifest_path);
        return 1;
    }

    // Determine manifest type from file extension
    std::string extension = file_path.extension().string();
    std::unique_ptr<ManifestParser> parser;

    if (extension == ".mpd") {
        logInfo("Detected MPD manifest file.");
        parser = std::make_unique<MPDManifestParser>();
    } else if (extension == ".m3u8") {
        logInfo("Detected M3U8 manifest file.");
        parser = std::make_unique<M3U8ManifestParser>();
    } else {
        logError("Unsupported manifest type: " + extension);
        return 1;
    }

    Application app(std::move(parser));
    if (!app.run(manifest_path)) {
        logError("Test application failed.");
        return 1;
    }

    logInfo("Application completed successfully.");
    return 0;
}
