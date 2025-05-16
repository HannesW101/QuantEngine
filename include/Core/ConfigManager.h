// Ensures the headers are only included once.
#pragma once

// Same project headers.
// ....
// 3rd party headers.
// ....
// std headers.
#include <string>
#include <map>

namespace QuantEngine {
    // Central configuration management for the quantitative engine
    // Handles loading and accessing API keys from configuration files
    class ConfigManager {
    public:
        // Returns the single global instance of the configuration manager
        static ConfigManager& getInstance();

        // Loads configuration data from JSON file
        // Default path is config.json in current directory
        // Throws exception if file can't be read or parsed
        void loadConfig(const std::string& configPath = "config.json");

        // Returns stored API key for the specified service
        // Returns empty string if service isn't found
        std::string getApiKey(const std::string& service) const;

    private:
        // Private constructor prevents external instantiation
        ConfigManager() = default;

        // Stores API keys as key-value pairs:
        // service name -> API key string
        std::map<std::string, std::string> apiKeys;
    };
}