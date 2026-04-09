#pragma once

#include <memory>
#include <optional>
#include <string>

#include <SDL2/SDL.h>

#include "launcher/core/assets.hpp"
#include "launcher/core/config.hpp"
#include "launcher/core/input.hpp"
#include "launcher/core/library.hpp"
#include "launcher/core/sdl_manager.hpp"
#include "launcher/core/state.hpp"
#include "launcher/ui/game_list.hpp"
#include "launcher/ui/main_menu.hpp"
#include "launcher/ui/splash.hpp"

namespace launcher {

class App {
public:
    explicit App(std::string configPath = "config.json");
    void run();

private:
    Config config_;
    SDL2Manager sdl2Manager_;
    Renderer renderer_;
    AssetManager assets_;

    GameLibrary gameLibrary_;
    MainMenu mainMenu_;
    GameList gameList_;
    SplashScreen splashScreen_;

    StateManager stateManager_;
    InputHandler inputHandler_;

    SDL_Texture* backgroundTexture_ = nullptr;
    bool running_ = true;

    void handleInput();
    void update(double deltaTime);
    void drawGlobalOverlays();
    void render();
    void launchGame(const std::string& romFilename);
};

}  // namespace launcher
