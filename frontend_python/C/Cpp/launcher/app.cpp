#include "launcher/app.hpp"

#include <SDL2/SDL_image.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "launcher/core/constants.hpp"

namespace launcher {

namespace {

std::string shellQuote(const std::string& value) {
    std::string quoted;
    quoted.reserve(value.size() + 2);
    quoted.push_back('\'');
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('\'');
    return quoted;
}

}  // namespace

App::App(std::string configPath)
    : config_(std::move(configPath)),
      sdl2Manager_(config_.windowWidth(), config_.windowHeight()),
      renderer_(sdl2Manager_),
      assets_(sdl2Manager_, renderer_, config_.assetsDir()),
      gameLibrary_(RomScanner(config_.romsDir()).scan()),
      mainMenu_(gameLibrary_),
      gameList_(gameLibrary_),
      splashScreen_(2.0),
      stateManager_("splash") {
    const auto wallpaperPath = std::filesystem::path("./resources/wallpaper.jpg");
    backgroundTexture_ = IMG_LoadTexture(renderer_.raw(), wallpaperPath.string().c_str());
    if (!backgroundTexture_) {
        std::cout << "[WARNING] Failed to load wallpaper: " << wallpaperPath << '\n';
    }
}

void App::handleInput() {
    const auto actions = inputHandler_.poll();

    for (const auto action : actions) {
        if (action == Action::Quit) {
            running_ = false;
            continue;
        }

        const auto& state = stateManager_.getState();

        if (state == "splash") {
            continue;
        }

        if (state == "main_menu") {
            if (action == Action::MoveLeft) {
                mainMenu_.moveLeft();
            } else if (action == Action::MoveRight) {
                mainMenu_.moveRight();
            } else if (action == Action::Select) {
                const auto selectedSystem = mainMenu_.getSelectedSystem();
                if (selectedSystem.has_value()) {
                    gameList_.setSystem(*selectedSystem);
                    stateManager_.transition("game_list");
                }
            } else if (action == Action::Back) {
                running_ = false;
            }
            continue;
        }

        if (state == "game_list") {
            if (action == Action::MoveUp) {
                gameList_.moveUp();
            } else if (action == Action::MoveDown) {
                gameList_.moveDown();
            } else if (action == Action::Select) {
                const auto selectedRom = gameList_.getSelectedRom();
                if (selectedRom.has_value()) {
                    launchGame(*selectedRom);
                }
            } else if (action == Action::Back) {
                stateManager_.transition("main_menu");
            }
        }
    }
}

void App::update(double deltaTime) {
    const auto& state = stateManager_.getState();
    if (state == "splash") {
        if (splashScreen_.update()) {
            stateManager_.transition("main_menu");
        }
    } else if (state == "main_menu") {
        mainMenu_.update(deltaTime);
    } else if (state == "game_list") {
        gameList_.update(deltaTime);
    }
}

void App::drawGlobalOverlays() {
    const int width = config_.windowWidth();
    const int height = config_.windowHeight();

    renderer_.drawRect(UI_MARGIN, UI_TOP_BAR_H, width - (UI_MARGIN * 2), 1, COLOR_NEON_CYAN, true);
    renderer_.drawRect(UI_MARGIN, height - UI_BOTTOM_BAR_H - 1, width - (UI_MARGIN * 2), 1, COLOR_BORDER_GLOW, true);

    renderer_.drawRect(UI_MARGIN, 2, width - (UI_MARGIN * 2), 1, COLOR_PANEL_ALT, true);
    renderer_.drawRect(UI_MARGIN, height - 3, width - (UI_MARGIN * 2), 1, COLOR_PANEL_ALT, true);
}

void App::render() {
    renderer_.clear();

    const auto& state = stateManager_.getState();

    if (backgroundTexture_) {
        renderer_.drawTexture(backgroundTexture_, 0, 0, config_.windowWidth(), config_.windowHeight());
        if (state != "splash") {
            drawGlobalOverlays();
        }
    }

    if (state == "splash") {
        splashScreen_.render(renderer_, assets_);
    } else if (state == "main_menu") {
        assets_.drawTextShadowed(renderer_, "SYSTEM SELECT", 14, 8, COLOR_TEXT_STRONG, COLOR_PANEL_ALT, 14, 1);
        mainMenu_.render(renderer_, assets_, gameLibrary_);
        assets_.drawTextShadowed(renderer_, "LEFT/RIGHT SWITCH  ENTER CONFIRM  ESC EXIT", 12, config_.windowHeight() - 20, COLOR_DIM, std::nullopt, 10, 1);
    } else if (state == "game_list") {
        const auto selectedRom = gameList_.getSelectedRom();
        if (selectedRom.has_value()) {
            const auto selectedSystem = gameList_.currentSystem().value_or("unknown");
            const auto selectedDisplaySystem = gameLibrary_.getSystemDisplayName(selectedSystem);
            const auto selectedTitle = AssetManager::truncateText(selectedDisplaySystem, 28);
            auto* selectedSprite = assets_.renderText(selectedTitle, COLOR_HIGHLIGHT, 14);
            if (selectedSprite) {
                const int centeredX = (config_.windowWidth() - selectedSprite->width) / 2;
                renderer_.drawTexture(selectedSprite->texture, centeredX, 10, selectedSprite->width, selectedSprite->height);
            } else {
                renderer_.drawRect(95, 10, 130, 12, COLOR_HIGHLIGHT, false);
            }
        }

        gameList_.render(renderer_, assets_);
        assets_.drawTextShadowed(renderer_, "UP/DOWN MOVE  ENTER LAUNCH  ESC BACK", 12, config_.windowHeight() - 20, COLOR_DIM, std::nullopt, 10, 1);
    }

    renderer_.present();
}

void App::run() {
    std::cout << "[INFO] Starting handheld launcher...\n";
    splashScreen_.start();

    const int frameTimeMs = 1000 / config_.targetFps();
    auto lastFrameTime = std::chrono::steady_clock::now();

    while (running_) {
        const auto frameStart = std::chrono::steady_clock::now();
        const std::chrono::duration<double> delta = frameStart - lastFrameTime;
        lastFrameTime = frameStart;

        handleInput();
        update(delta.count());
        render();

        const auto frameEnd = std::chrono::steady_clock::now();
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();
        const auto remainingMs = frameTimeMs - static_cast<int>(elapsedMs);
        if (remainingMs > 0) {
            SDL_Delay(static_cast<Uint32>(remainingMs));
        }
    }

    if (backgroundTexture_) {
        SDL_DestroyTexture(backgroundTexture_);
        backgroundTexture_ = nullptr;
    }

    std::cout << "[INFO] Goodbye!\n";
}

void App::launchGame(const std::string& romFilename) {
    const auto selectedSystem = gameList_.currentSystem();
    if (!selectedSystem.has_value()) {
        std::cout << "[ERROR] Cannot launch game: no system selected.\n";
        return;
    }

    const auto coreFilename = config_.coreForSystem(*selectedSystem);
    if (!coreFilename.has_value()) {
        std::cout << "[ERROR] Cannot launch game: no core mapping configured for system '" << *selectedSystem << "'.\n";
        return;
    }

    const auto corePath = config_.coresDir() / *selectedSystem / *coreFilename;
    const auto romPath = config_.romsDir() / *selectedSystem / romFilename;

    if (!std::filesystem::exists(corePath)) {
        std::cout << "[ERROR] Cannot launch game: core file not found: " << corePath << '\n';
        return;
    }

    if (!std::filesystem::exists(romPath)) {
        std::cout << "[ERROR] Cannot launch game: ROM file not found: " << romPath << '\n';
        return;
    }

    const std::string command =
        shellQuote(config_.retroarchBin()) +
        " -L " + shellQuote(corePath.string()) +
        " " + shellQuote(romPath.string());

    std::cout << "[INFO] Launching: " << command << '\n';
    const int exitCode = std::system(command.c_str());
    if (exitCode != 0) {
        std::cout << "[ERROR] RetroArch exited with code " << exitCode << ".\n";
    }
}

}  // namespace launcher
