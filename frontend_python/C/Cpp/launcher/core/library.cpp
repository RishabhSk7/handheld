#include "launcher/core/library.hpp"

#include <algorithm>
#include <iostream>

#include "launcher/core/constants.hpp"

namespace launcher {

GameLibrary::GameLibrary(std::unordered_map<std::string, std::vector<std::string>> systems)
    : systems_(std::move(systems)) {
    systemNames_.reserve(systems_.size());
    for (const auto& [name, _] : systems_) {
        systemNames_.push_back(name);
    }
    std::sort(systemNames_.begin(), systemNames_.end());
}

const std::vector<std::string>& GameLibrary::getSystems() const {
    return systemNames_;
}

std::vector<std::string> GameLibrary::getRoms(const std::string& system) const {
    auto it = systems_.find(system);
    if (it == systems_.end()) {
        return {};
    }
    auto roms = it->second;
    std::sort(roms.begin(), roms.end());
    return roms;
}

std::string GameLibrary::getSystemDisplayName(const std::string& system) const {
    if (system == "nes") return "NES";
    if (system == "snes") return "SNES";
    if (system == "gba") return "GBA";
    if (system == "gb") return "Game Boy";
    if (system == "gbc") return "GBC";

    std::string upper = system;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return upper;
}

int GameLibrary::getRomCount() const {
    int total = 0;
    for (const auto& [_, roms] : systems_) {
        total += static_cast<int>(roms.size());
    }
    return total;
}

RomScanner::RomScanner(std::filesystem::path romsDir)
    : romsDir_(std::move(romsDir)) {
    std::filesystem::create_directories(romsDir_);
}

GameLibrary RomScanner::scan() const {
    std::unordered_map<std::string, std::vector<std::string>> systems;

    for (const auto& entry : std::filesystem::directory_iterator(romsDir_)) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto systemName = entry.path().filename().string();
        std::vector<std::string> roms;

        for (const auto& romEntry : std::filesystem::directory_iterator(entry.path())) {
            if (!romEntry.is_regular_file()) {
                continue;
            }

            const auto filename = romEntry.path().filename().string();
            if (!filename.empty() && filename[0] == '.') {
                continue;
            }

            std::string ext = romEntry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

            const bool valid = std::any_of(
                VALID_ROM_EXTENSIONS.begin(),
                VALID_ROM_EXTENSIONS.end(),
                [&ext](std::string_view candidate) { return ext == candidate; }
            );

            if (valid) {
                roms.push_back(filename);
            }
        }

        if (!roms.empty()) {
            std::cout << "[INFO] Found system '" << systemName << "' with " << roms.size() << " ROM(s)\n";
            systems[systemName] = roms;
        }
    }

    if (systems.empty()) {
        std::cout << "[WARNING] No ROMs found. Library is empty.\n";
    }

    return GameLibrary(std::move(systems));
}

}  // namespace launcher
