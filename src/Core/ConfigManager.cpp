// Same project headers.
#include "Core/ConfigManager.h"
// 3rd party headers.
#include "nlohmann/json.hpp"
// std headers.
#include <stdexcept>
#include <fstream>

namespace QuantEngine {
    // Singleton access point - creates single instance on first call
    // Thread-safe in C++11 and later (static local initialization)
    ConfigManager& ConfigManager::getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // Loads configuration from JSON file
    // Default path is config.json in executable directory
    // Expects format: { "api_keys": { "service1": "key1", ... } }
    // Throws if file missing or malformed JSON
    void ConfigManager::loadConfig(const std::string& configPath) {
        std::ifstream configFile(configPath);
        nlohmann::json config;
        configFile >> config;  // Parse entire JSON file

        // Extract all API keys from "api_keys" section
        for (auto& [key, value] : config["api_keys"].items()) {
            apiKeys[key] = value.get<std::string>();
        }
    }

    // Retrieves stored API key for specified service
    // Case-sensitive service name matching JSON keys
    // Throws if service isn't found in loaded configuration
    std::string ConfigManager::getApiKey(const std::string& service) const {
        auto it = apiKeys.find(service);
        if (it != apiKeys.end()) {
            return it->second;
        }
        throw std::runtime_error("API key not found for service: " + service);
    }
}