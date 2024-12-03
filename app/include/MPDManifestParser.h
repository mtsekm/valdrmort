#include "ManifestParser.h"

#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>

class MPDManifestParser : public ManifestParser {

public:
    bool parse(const std::string &manifest_path) override;

private:
    // Helper methods to parse MPD XML
    bool parseMPDXML(const std::string &manifest_content);
    void extractDRMKeys(xmlNodePtr root);
    void extractStreamURI(xmlNodePtr root);
};
