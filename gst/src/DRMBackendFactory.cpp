#include "DRMBackendFactory.h"
#include "logger.h"

// TODO Include specific DRM backend implementations
#include "ClearKeyBackend.h"

#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, std::string> system_id_to_backend = {
    {CLEARKEY_PROTECTION_ID, "ClearKey"},
    {WIDEVINE_PROTECTION_ID, "Widevine"},
    {PLAYREADY_PROTECTION_ID, "Playready"},
    {FAIRPLAY_PROTECTION_ID, "Fairplay"}
    // TODO Add more mappings here
};

std::unique_ptr<DRMBackend> DRMBackendFactory::create(const std::string &system_id) {
    LOG_INFO("Creating DRM backend for system ID: %s", system_id.c_str());

    auto it = system_id_to_backend.find(system_id);
    if (it == system_id_to_backend.end()) {
        LOG_ERROR("Unknown DRM system ID: %s", system_id.c_str());
        throw std::invalid_argument("Unknown DRM system ID: " + system_id);
    }

    const std::string &backend_name = it->second;

    if (backend_name == "ClearKey") {
        LOG_DEBUG("Creating ClearKey DRM backend");
        return std::make_unique<ClearKeyBackend>();
    }
    // TODO Add more backend creation logic here

    LOG_ERROR("Unsupported DRM backend: %s", backend_name.c_str());
    throw std::invalid_argument("Unsupported DRM backend: " + backend_name);
}