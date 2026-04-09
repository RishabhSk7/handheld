#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "launcher/core/assets.hpp"
#include "launcher/core/library.hpp"
#include "launcher/core/sdl_manager.hpp"

namespace launcher {

class GameList {
public:
    explicit GameList(const GameLibrary& gameLibrary);

    void setSystem(const std::string& system);
    void moveUp();
    void moveDown();
    void update(double deltaTime);

    std::optional<std::string> getSelectedRom() const;
    std::vector<std::tuple<std::string, bool, double>> getVisibleRoms() const;
    void render(Renderer& renderer, AssetManager& assets);

    std::optional<std::string> currentSystem() const;

private:
    const GameLibrary& gameLibrary_;
    std::optional<std::string> currentSystem_;
    std::vector<std::string> roms_;
    int currentIndex_ = 0;
    int targetIndex_ = 0;
    double offset_ = 0.0;
    double smoothing_ = 8.0;
    int itemsVisible_ = 5;
};

}  // namespace launcher
