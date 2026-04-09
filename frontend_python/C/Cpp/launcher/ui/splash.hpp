#pragma once

#include "launcher/core/assets.hpp"
#include "launcher/core/sdl_manager.hpp"

namespace launcher {

class SplashScreen {
public:
    explicit SplashScreen(double durationSeconds = 2.0);

    void start();
    bool update();
    void render(Renderer& renderer, AssetManager& assets);

private:
    double durationSeconds_;
    Uint32 startTicks_ = 0;
    bool started_ = false;
};

}  // namespace launcher
