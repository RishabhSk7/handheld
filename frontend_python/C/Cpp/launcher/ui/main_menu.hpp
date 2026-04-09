#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "launcher/core/assets.hpp"
#include "launcher/core/library.hpp"
#include "launcher/core/sdl_manager.hpp"

namespace launcher {

class MainMenu {
public:
    explicit MainMenu(const GameLibrary& gameLibrary);

    void moveLeft();
    void moveRight();
    void update(double deltaTime);
    std::optional<std::string> getSelectedSystem() const;
    std::vector<std::tuple<std::string, bool, double>> getVisibleSystems() const;
    void render(Renderer& renderer, AssetManager& assets, const GameLibrary& gameLibrary);

private:
    std::vector<std::string> systems_;
    int currentIndex_ = 0;
    int targetIndex_ = 0;
    double offset_ = 0.0;
    double smoothing_ = 8.0;
};

}  // namespace launcher
