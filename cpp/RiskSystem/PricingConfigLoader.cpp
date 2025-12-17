/*#include "PricingConfigLoader.h"
#include <stdexcept>

std::string PricingConfigLoader::getConfigFile() const {
    return configFile_;
}

void PricingConfigLoader::setConfigFile(const std::string& file) {
    configFile_ = file;
}

PricingEngineConfig PricingConfigLoader::loadConfig() {
    throw std::runtime_error("Not implemented");
}*/

#include "PricingConfigLoader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

/*
 * PricingConfigLoader
 *
 * Goal:
 *  - Load the PricingEngines.xml configuration and expose it as a
 *    PricingEngineConfig (a vector-like collection of PricingEngineConfigItem).
 *
 * Design for MAX REUSE:
 *  - We split the work into two reusable steps:
 *      (1) loadConfig(): file I/O only (reads the file into a string)
 *      (2) parseXml(): parsing only (converts XML text -> config objects)
 *
 *  This "read file -> parse content" pattern can be reused for other config
 *  loaders (market data config, curve config, etc.) without duplicating I/O code.
 *
 * XML note:
 *  - We avoid external XML dependencies (e.g., tinyxml2) because the project
 *    may not ship with them.
 *  - The provided XML is flat and predictable:
 *      <PricingEngines>
 *        <Engine tradeType="..." assembly="..." pricingEngine="..." />
 *        ...
 *      </PricingEngines>
 *  - For this controlled structure, attribute extraction is sufficient.
 */

std::string PricingConfigLoader::getConfigFile() const {
    return configFile_;
}

void PricingConfigLoader::setConfigFile(const std::string& file) {
    configFile_ = file;
}

PricingEngineConfig PricingConfigLoader::loadConfig() {
    // Reuse-friendly: loader validates its inputs early and fails fast.
    if (configFile_.empty()) {
        throw std::invalid_argument("Config file cannot be empty");
    }

    // Step 1 (I/O): read the entire XML file as text.
    // Keeping this separate makes it easy to:
    //  - unit test parsing without touching the filesystem
    //  - reuse the same I/O pattern across other config loaders
    std::ifstream in(configFile_);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open config file: " + configFile_);
    }

    std::ostringstream oss;
    oss << in.rdbuf();

    // Step 2 (Parsing): turn XML string into structured config objects.
    return parseXml(oss.str());
}

PricingEngineConfig PricingConfigLoader::parseXml(const std::string& content) {
    PricingEngineConfig config;

    // Minimal XML parsing approach:
    // We look for lines containing "<Engine" and then extract attributes.
    // This is intentionally simple because:
    //  - the input format is controlled by the exercise
    //  - there are no nested elements inside <Engine ... />
    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
        // Only process Engine entries
        if (line.find("<Engine") == std::string::npos) {
            continue;
        }

        // Small reusable helper: extract attribute values from a single Engine line.
        // If you later build other config loaders, you can reuse the same pattern.
        auto extractAttr = [&](const std::string& attr) -> std::string {
            const std::string key = attr + "=\"";
            auto pos = line.find(key);
            if (pos == std::string::npos) {
                throw std::runtime_error("Engine element missing attribute: " + attr);
            }

            pos += key.size();
            auto end = line.find('"', pos);
            if (end == std::string::npos || end < pos) {
                throw std::runtime_error("Malformed attribute value for: " + attr);
            }

            return line.substr(pos, end - pos);
        };

        // Convert XML -> strongly typed config item (separation of concerns).
        // IMPORTANT: we do NOT instantiate pricing engines here.
        // This loader only provides configuration; a factory elsewhere uses it.
        PricingEngineConfigItem item;
        item.setTradeType(extractAttr("tradeType"));
        item.setAssembly(extractAttr("assembly"));
        item.setTypeName(extractAttr("pricingEngine"));

        config.push_back(item);
    }

    return config;
}
