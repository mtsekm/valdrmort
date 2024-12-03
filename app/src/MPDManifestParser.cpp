#include "MPDManifestParser.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

bool MPDManifestParser::parse(const std::string &manifest_path) {
    logInfo("Parsing MPD manifest: " + manifest_path);

    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        logError("Failed to open MPD manifest file: " + manifest_path);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    return parseMPDXML(content);
}

bool MPDManifestParser::parseMPDXML(const std::string &manifest_content) {
    xmlDocPtr doc = xmlReadMemory(manifest_content.c_str(), manifest_content.size(), "noname.xml", nullptr, 0);
    if (!doc) {
        logError("Failed to parse MPD XML");
        return false;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root || xmlStrcmp(root->name, BAD_CAST "MPD")) {
        logError("Invalid MPD manifest: Missing MPD root element");
        xmlFreeDoc(doc);
        return false;
    }

    extractDRMKeys(root);
    extractStreamURI(root);

    xmlFreeDoc(doc);
    logInfo("Successfully parsed MPD manifest");
    return true;
}

void MPDManifestParser::extractDRMKeys(xmlNodePtr root) {
    for (xmlNodePtr child = root->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, BAD_CAST "ContentProtection") == 0) {
            xmlChar *scheme_id = xmlGetProp(child, BAD_CAST "schemeIdUri");
            if (scheme_id) {
                DRMKey key;
                key.system_id = reinterpret_cast<const char *>(scheme_id);
                key.key_uri = ""; // Extract URI if available
                drm_keys.push_back(key);
                logDebug("Found DRM key: " + key.system_id);
                xmlFree(scheme_id);
            }
        }
    }
}

void MPDManifestParser::extractStreamURI(xmlNodePtr root) {
    for (xmlNodePtr child = root->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, BAD_CAST "BaseURL") == 0) {
            xmlChar *uri = xmlNodeGetContent(child);
            if (uri) {
                stream_uri = reinterpret_cast<const char *>(uri);
                logDebug("Found stream URI: ", stream_uri);
                xmlFree(uri);
            }
        }
    }
}
