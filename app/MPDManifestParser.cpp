#include "MPDManifestParser.h"
#include "logger.h"

#include <fstream>
#include <sstream>

bool MPDManifestParser::parse(const std::string &manifest_path) {
    LOG_INFO("Parsing MPD manifest: %s", manifest_path.c_str());

    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open MPD manifest file: %s", manifest_path.c_str());
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
        LOG_ERROR("Failed to parse MPD XML");
        return false;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root || xmlStrcmp(root->name, BAD_CAST "MPD")) {
        LOG_ERROR("Invalid MPD manifest: Missing MPD root element");
        xmlFreeDoc(doc);
        return false;
    }

    extractDRMKeys(root);
    extractStreamURI(root);

    xmlFreeDoc(doc);
    LOG_INFO("Successfully parsed MPD manifest");
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
                LOG_DEBUG("Found DRM key: %s", key.system_id.c_str());
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
                LOG_DEBUG("Found stream URI: %s", stream_uri.c_str());
                xmlFree(uri);
            }
        }
    }
}
