#ifndef DRM_BACKEND_FACTORY_H
#define DRM_BACKEND_FACTORY_H

#include "DRMBackend.h"

#include <memory>
#include <string>

// TODO add DRM system IDs here.
#define CLEARKEY_PROTECTION_ID "e2719d58-a985-b3c9-781a-b030af78d30e"
#define WIDEVINE_PROTECTION_ID "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
#define PLAYREADY_PROTECTION_ID "9a04f079-9840-4286-ab92-e65be0885f95"
#define FAIRPLAY_PROTECTION_ID "94ce86fb-07ff-4f43-adb8-93d2fa968ca2"

class DRMBackendFactory {
public:
    /**
     * Factory method to create a DRM backend based on the system ID.
     *
     * @param system_id The unique DRM system ID.
     *
     * @return A unique pointer to the appropriate DRM backend.
     */
    static std::unique_ptr<DRMBackend> create(const std::string &system_id);
};

#endif // DRM_BACKEND_FACTORY_H
