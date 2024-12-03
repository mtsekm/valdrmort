#include "M3U8ManifestParser.h"
#include "Logger.h"

#include <fstream>
#include <regex>
#include <sstream>

bool M3U8ManifestParser::parse(const std::string &manifest_path) {
    logInfo("Parsing M3U8 manifest: " + manifest_path);

    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        logError("Failed to open M3U8 manifest file: " + manifest_path);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    return parseM3U8(content);
}

bool M3U8ManifestParser::parseM3U8(const std::string &manifest_content) {
    std::istringstream stream(manifest_content);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("#EXT-X-KEY") != std::string::npos) {
            extractDRMKeys(line);
        } else if (!line.empty() && line[0] != '#') {
            stream_uri = line;
            logDebug("Found stream URI: " + stream_uri);
        }
    }

    logInfo("Successfully parsed M3U8 manifest");
    return true;
}

void M3U8ManifestParser::extractDRMKeys(const std::string &line) {
    std::regex key_regex(R"(URI=\"(.*?)\".*?KEYID=0x([0-9a-fA-F]{32}))");
    std::smatch match;

    if (std::regex_search(line, match, key_regex)) {
        DRMKey key;
        key.key_uri = match[1];
        key.system_id = match[2];
        drm_keys.push_back(key);
        logDebug("Found DRM key: " + key.system_id + " with URI: " + key.key_uri);
    }
}
