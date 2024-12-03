#pragma once

#include <string>
#include <vector>

struct DRMKey {
    std::string system_id; // DRM system ID (e.g., Widevine, PlayReady)
    std::string key_uri;   // URI to fetch the key
};

class ManifestParser {
public:
    virtual ~ManifestParser() = default;

    // Parses the manifest file and extracts DRM keys
    virtual bool parse(const std::string &manifest_path) = 0;

    // Returns the extracted DRM keys
    std::vector<DRMKey> getDRMKeys() const { return drm_keys; }

    // Returns the media stream URI (video/audio source)
    std::string getStreamURI() const { return stream_uri; }

protected:
    std::vector<DRMKey> drm_keys;
    std::string stream_uri;
};
