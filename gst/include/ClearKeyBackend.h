#ifndef CLEARKEY_BACKEND_H
#define CLEARKEY_BACKEND_H

#include "DRMBackend.h"

#include <string>
#include <vector>

class ClearKeyBackend : public DRMBackend {
public:
    explicit ClearKeyBackend();
    ~ClearKeyBackend();

protected:
    bool doInitialize() override;
    bool doDecrypt(const std::vector<uint8_t> &encrypted_data, std::vector<uint8_t> &decrypted_data) override;
    void doCleanup() override;

private:
    std::string key;
};

#endif // CLEARKEY_BACKEND_H
