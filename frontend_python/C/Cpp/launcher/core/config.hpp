#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace launcher {

class Config {
public:
    explicit Config(std::string configPath = "config.json");

    std::filesystem::path romsDir() const;
    std::filesystem::path coresDir() const;
    std::filesystem::path assetsDir() const;
    std::string retroarchBin() const;
    std::optional<std::string> coreForSystem(const std::string& system) const;

    int windowWidth() const;
    int windowHeight() const;
    int targetFps() const;

private:
    std::string configPath_;
    std::filesystem::path configDir_;
    std::unordered_map<std::string, std::string> values_;
    std::unordered_map<std::string, std::string> coreMap_;

    void loadConfig();
    static std::filesystem::path expandUser(const std::string& value);
    std::filesystem::path resolveRelativeToConfig(const std::filesystem::path& path) const;
    std::string getString(const std::string& key, const std::string& fallback) const;
    int getInt(const std::string& key, int fallback) const;
};

}  // namespace launcher
