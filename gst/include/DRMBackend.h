#ifndef DRM_BACKEND_H
#define DRM_BACKEND_H

#include "SecAPIManager.h"
#include "SecurityCompliance.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * Abstract DRMBackend interface.
 */
class DRMBackend {
public:
    explicit DRMBackend(const std::string &drm_type);
    virtual ~DRMBackend();

    /**
     * Performs security checks and initializes the backend.
     *
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * Acquires a license from the license server.
     *
     * @return true if the license was successfully acquired, false otherwise.
     */
    bool acquireLicense();

    /**
     * Decrypts data after ensuring key availability and output protection.
     *
     * @param encrypted_data The encrypted input data.
     * @param decrypted_data Output buffer to hold the decrypted data.
     *
     * @return true if decryption was successful, false otherwise.
     */
    bool decrypt(const std::vector<uint8_t> &encrypted_data, std::vector<uint8_t> &decrypted_data);

    /**
     * Cleans up resources used by the backend.
     */
    void cleanup();

protected:
    std::string drm_type; // DRM type (e.g., Widevine, PlayReady, ClearKey)

    std::map<std::string, std::string> key_map;

    SecurityCompliance security;                  // Security management instance
    std::unique_ptr<SecAPIManager> secapiManager; // SecAPI manager for cryptographic operations

    /**
     * Backend-specific initialization logic to be implemented by derived classes.
     */
    virtual bool doInitialize() = 0;

    /**
     * Backend-specific decryption logic to be implemented by derived classes.
     *
     * @param encrypted_data The encrypted input data.
     * @param decrypted_data Output buffer to hold the decrypted data.
     * @param key The decryption key to use.
     *
     * @return true if decryption was successful, false otherwise.
     */
    virtual bool doDecrypt(const std::vector<uint8_t> &encrypted_data, std::vector<uint8_t> &decrypted_data) = 0;

    /**
     * Backend-specific cleanup logic to be implemented by derived classes.
     */
    virtual void doCleanup() = 0;

private:
    bool initialized = false;      // Flag to track initialization state
    sa_key license_key_handle = 0; // Secure key handle for content decryption
};

#endif // DRM_BACKEND_H
