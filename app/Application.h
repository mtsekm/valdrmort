#pragma once

#include "ManifestParser.h"
#include "PipelineManager.h"
#include "LicenseRequestHelper.h"
#include <memory>

class Application {
private:
    std::unique_ptr<ManifestParser> parser;
    PipelineManager pipeline_manager;
    LicenseRequestHelper license_helper;

public:
    explicit Application(std::unique_ptr<ManifestParser> parser);

    // Main workflow
    bool run(const std::string &manifest_path);
};
