#include "DRMBackend.h"
#include "logger.h"

#include <fstream>
#include <sstream>

DRMBackend::DRMBackend(const std::string &drm_type)
    : drm_type(drm_type), secapiManager(std::make_unique<SecAPIManager>()) {}

DRMBackend::~DRMBackend() = default;

bool DRMBackend::initialize() {
    if (initialized) {
        LOG_WARN("DRMBackend: Already initialized!");
        return false;
    }

    initialized = doInitialize();
    if (!initialized) {
        LOG_ERROR("DRMBackend: Initialization failed!");
        return false;
    }

    LOG_INFO("DRMBackend: Initialized successfully.");
    return true;
}

bool DRMBackend::decrypt(const std::vector<uint8_t> &encrypted_data, std::vector<uint8_t> &decrypted_data) {
    if (!initialized) {
        LOG_ERROR("DRMBackend: Backend not initialized!");
        return false;
    }

    // if (!license_key_handle) {
    //     LOG_ERROR("DRMBackend: No valid secure key handle available!");
    //     return false;
    // }

    doDecrypt(encrypted_data, decrypted_data);

    LOG_INFO("DRMBackend: Decryption successful.");
    return true;
}

void DRMBackend::cleanup() {
    if (!initialized) {
        LOG_WARN("DRMBackend: Not initialized, nothing to clean up.");
        return;
    }

    doCleanup();

    initialized = false;
    LOG_INFO("DRMBackend: Resources cleaned up.");
}
