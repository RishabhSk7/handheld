#pragma once

#include <SDL2/SDL.h>

#include <stdexcept>
#include <string>

namespace launcher {

class SDL2Manager {
public:
    SDL2Manager(int width, int height);
    ~SDL2Manager();

    SDL2Manager(const SDL2Manager&) = delete;
    SDL2Manager& operator=(const SDL2Manager&) = delete;

    SDL_Window* window() const;
    SDL_Renderer* renderer() const;

private:
    int width_;
    int height_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    void init();
};

class Renderer {
public:
    explicit Renderer(SDL2Manager& manager);

    void clear();
    void present();
    void drawRect(int x, int y, int width, int height, const SDL_Color& color, bool filled = true);
    void drawTexture(SDL_Texture* texture, int x, int y, int width, int height, bool cropToSquare = false);
    SDL_Renderer* raw() const;

private:
    SDL2Manager& manager_;
    SDL_Renderer* renderer_;
};

}  // namespace launcher
