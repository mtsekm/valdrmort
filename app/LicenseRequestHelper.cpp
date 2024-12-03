#include "LicenseRequestHelper.h"
#include "logger.h"

#include <cjson/cJSON.h>
#include <curl/curl.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

LicenseRequestHelper::LicenseRequestHelper() { curl_global_init(CURL_GLOBAL_DEFAULT); }

LicenseRequestHelper::~LicenseRequestHelper() { curl_global_cleanup(); }

std::string LicenseRequestHelper::encodeKeyIdToBase64Url(const std::string &raw_key_id) {
    // Convert GUID to big-endian byte array
    std::vector<uint8_t> key_bytes(16);
    for (size_t i = 0; i < raw_key_id.size(); i += 2) {
        key_bytes[i / 2] = std::stoi(raw_key_id.substr(i, 2), nullptr, 16);
    }
    std::reverse(key_bytes.begin(), key_bytes.end());

    // Base64 URL-safe encoding
    static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string encoded;
    int val = 0, valb = -6;
    for (uint8_t c : key_bytes) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    return encoded;
}

std::vector<uint8_t> LicenseRequestHelper::decodeOctetKey(const std::string &encoded_key) {
    std::string padded_key = encoded_key;
    while (padded_key.size() % 4) {
        padded_key.push_back('=');
    }
    std::vector<uint8_t> decoded;
    auto decode_char = [](char c) -> int {
        if (c >= 'A' && c <= 'Z')
            return c - 'A';
        if (c >= 'a' && c <= 'z')
            return c - 'a' + 26;
        if (c >= '0' && c <= '9')
            return c - '0' + 52;
        if (c == '-')
            return 62;
        if (c == '_')
            return 63;
        return -1;
    };

    int val = 0, valb = -8;
    for (char c : padded_key) {
        if (c == '=')
            break;
        int d = decode_char(c);
        if (d == -1)
            throw std::invalid_argument("Invalid Base64 character");
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return decoded;
}

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    auto *response = static_cast<std::string *>(userp);
    size_t total_size = size * nmemb;
    response->append(static_cast<char *>(contents), total_size);
    return total_size;
}

bool LicenseRequestHelper::sendPostRequest(const std::string &url, const std::string &drm_message,
                                           const std::map<std::string, std::string> &key_map, std::string &key) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        LOG_ERROR("LicenseRequestHelper: Failed to initialize cURL.");
        return false;
    }

    // Prepare headers
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string drm_header = "X-AxDRM-Message: " + drm_message;
    headers = curl_slist_append(headers, drm_header.c_str());

    // Create JSON payload
    cJSON *payload = cJSON_CreateObject();
    cJSON *kids = cJSON_AddArrayToObject(payload, "kids");

    for (const auto &[key_id, _] : key_map) {
        cJSON_AddItemToArray(kids, cJSON_CreateString(encodeKeyIdToBase64Url(key_id).c_str()));
    }

    char *json_payload = cJSON_PrintUnformatted(payload);
    cJSON_Delete(payload);

    // Send POST request
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        logCurlError("sendPostRequest", res);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(json_payload);
        return false;
    }

    // Parse JSON response
    cJSON *response_json = cJSON_Parse(response.c_str());
    if (!response_json) {
        LOG_ERROR("LicenseRequestHelper: Failed to parse JSON response.");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(json_payload);
        return false;
    }

    cJSON *keys_array = cJSON_GetObjectItem(response_json, "keys");
    if (keys_array && cJSON_IsArray(keys_array)) {
        for (cJSON *key_item = keys_array->child; key_item; key_item = key_item->next) {
            cJSON *kid = cJSON_GetObjectItem(key_item, "kid");
            cJSON *k = cJSON_GetObjectItem(key_item, "k");
            if (kid && cJSON_IsString(kid) && k && cJSON_IsString(k)) {
                auto decoded_key = decodeOctetKey(k->valuestring);
                key.assign(reinterpret_cast<const char *>(decoded_key.data()), decoded_key.size());
                break;
            }
        }
    }

    cJSON_Delete(response_json);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_payload);

    return !key.empty();
}

void LicenseRequestHelper::logCurlError(const std::string &operation, int error_code) {
    LOG_ERROR("LicenseRequestHelper: %s failed with cURL error: %s", operation.c_str(),
              curl_easy_strerror(static_cast<CURLcode>(error_code)));
}
