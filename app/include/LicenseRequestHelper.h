#ifndef LICENSE_REQUEST_HELPER_H
#define LICENSE_REQUEST_HELPER_H

#include <map>
#include <string>
#include <vector>

class LicenseRequestHelper {
public:
    LicenseRequestHelper();
    ~LicenseRequestHelper();

    /**
     * Encode a Key ID to a Base64 URL-safe format.
     *
     * @param raw_key_id The raw Key ID (e.g., GUID format).
     * @return The Base64 URL-safe encoded Key ID.
     */
    std::string encodeKeyIdToBase64Url(const std::string &raw_key_id);

    /**
     * Decode an octet key from Base64 URL-safe format.
     *
     * @param encoded_key The encoded Base64 URL-safe key.
     * @return The decoded raw key.
     */
    std::vector<uint8_t> decodeOctetKey(const std::string &encoded_key);

    /**
     * Send a POST request to a license server.
     *
     * @param url The license server URL.
     * @param drm_message The DRM message for the license request.
     * @param key_map A map of key IDs and values.
     * @param key Output parameter to hold the acquired key.
     * @return true if the license was acquired successfully, false otherwise.
     */
    bool sendPostRequest(const std::string &url, const std::string &drm_message,
                         const std::map<std::string, std::string> &key_map, std::string &key);

private:
    /**
     * Log cURL errors with context.
     *
     * @param operation The operation being logged.
     * @param error_code The cURL error code.
     */
    void logCurlError(const std::string &operation, int error_code);
};

#endif // LICENSE_REQUEST_HELPER_H
