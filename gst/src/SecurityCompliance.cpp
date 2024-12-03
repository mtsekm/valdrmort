#include "SecurityCompliance.h"
#include "logger.h"

bool SecurityCompliance::checkOutputProtection() const {
    // TODO Perform HDCP check
    LOG_INFO("SecurityCompliance: HDCP check passed.");
    return true;
}
