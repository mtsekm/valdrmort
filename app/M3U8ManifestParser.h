#include "ManifestParser.h"

#include <string>
#include <vector>

class M3U8ManifestParser : public ManifestParser {

public:
    bool parse(const std::string &manifest_path) override;

private:
    // Helper methods to parse M3U8 content
    bool parseM3U8(const std::string &manifest_content);
    void extractDRMKeys(const std::string &content);
    void extractStreamURI(const std::string &content);
};
