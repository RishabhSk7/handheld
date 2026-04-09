#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace launcher {

class GameLibrary {
public:
    explicit GameLibrary(std::unordered_map<std::string, std::vector<std::string>> systems);

    const std::vector<std::string>& getSystems() const;
    std::vector<std::string> getRoms(const std::string& system) const;
    std::string getSystemDisplayName(const std::string& system) const;
    int getRomCount() const;

private:
    std::unordered_map<std::string, std::vector<std::string>> systems_;
    std::vector<std::string> systemNames_;
};

class RomScanner {
public:
    explicit RomScanner(std::filesystem::path romsDir);

    GameLibrary scan() const;

private:
    std::filesystem::path romsDir_;
};

}  // namespace launcher
