#include "launcher/core/config.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

namespace launcher {

Config::Config(std::string configPath) : configPath_(std::move(configPath)) {
    loadConfig();
}

void Config::loadConfig() {
    std::filesystem::path selectedPath;

    const std::filesystem::path rawPath(configPath_);
    if (rawPath.is_absolute() && std::filesystem::exists(rawPath)) {
        selectedPath = rawPath;
    } else {
        const auto cwd = std::filesystem::current_path();
        const std::vector<std::filesystem::path> candidates = {
            rawPath,
            cwd / rawPath,
            cwd.parent_path() / rawPath,
            cwd.parent_path().parent_path() / rawPath,
        };

        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                selectedPath = candidate;
                break;
            }
        }
    }

    if (selectedPath.empty()) {
        throw std::runtime_error("Config file not found: " + configPath_);
    }

    configDir_ = selectedPath.parent_path();
    std::ifstream file(selectedPath);
    if (!file) {
        throw std::runtime_error("Config file not found: " + configPath_);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    const std::string content = buffer.str();

    coreMap_.clear();

    const std::regex stringPattern(R"rx("([^"]+)"\s*:\s*"([^"]*)")rx");
    const std::regex intPattern(R"rx("([^"]+)"\s*:\s*(-?\d+))rx");

    for (auto it = std::sregex_iterator(content.begin(), content.end(), stringPattern);
         it != std::sregex_iterator(); ++it) {
        values_[(*it)[1].str()] = (*it)[2].str();
    }

    for (auto it = std::sregex_iterator(content.begin(), content.end(), intPattern);
         it != std::sregex_iterator(); ++it) {
        values_[(*it)[1].str()] = (*it)[2].str();
    }

    const std::regex coreMapPattern(R"rx("core_map"\s*:\s*\{([^}]*)\})rx");
    std::smatch coreMapMatch;
    if (std::regex_search(content, coreMapMatch, coreMapPattern)) {
        const std::string block = coreMapMatch[1].str();
        for (auto it = std::sregex_iterator(block.begin(), block.end(), stringPattern);
             it != std::sregex_iterator(); ++it) {
            coreMap_[(*it)[1].str()] = (*it)[2].str();
        }
    }
}

std::filesystem::path Config::expandUser(const std::string& value) {
    if (!value.empty() && value[0] == '~') {
        const char* home = std::getenv("HOME");
        if (home) {
            if (value.size() == 1) {
                return std::filesystem::path(home);
            }
            if (value[1] == '/') {
                return std::filesystem::path(home) / value.substr(2);
            }
        }
    }
    return std::filesystem::path(value);
}

std::filesystem::path Config::resolveRelativeToConfig(const std::filesystem::path& path) const {
    if (path.is_absolute()) {
        return path;
    }
    if (configDir_.empty()) {
        return path;
    }
    return configDir_ / path;
}

std::string Config::getString(const std::string& key, const std::string& fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return fallback;
    }
    return it->second;
}

int Config::getInt(const std::string& key, int fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return fallback;
    }
    try {
        return std::stoi(it->second);
    } catch (...) {
        return fallback;
    }
}

std::filesystem::path Config::romsDir() const {
    return resolveRelativeToConfig(expandUser(getString("roms_dir", "./roms")));
}

std::filesystem::path Config::coresDir() const {
    return resolveRelativeToConfig(expandUser(getString("cores_dir", "./cores")));
}

std::filesystem::path Config::assetsDir() const {
    return resolveRelativeToConfig(expandUser(getString("assets_dir", "./assets")));
}

std::string Config::retroarchBin() const {
    return getString("retroarch_bin", "retroarch");
}

std::optional<std::string> Config::coreForSystem(const std::string& system) const {
    const auto it = coreMap_.find(system);
    if (it == coreMap_.end()) {
        return std::nullopt;
    }
    return it->second;
}

int Config::windowWidth() const {
    return getInt("window_width", 320);
}

int Config::windowHeight() const {
    return getInt("window_height", 240);
}

int Config::targetFps() const {
    return getInt("target_fps", 60);
}

}  // namespace launcher
