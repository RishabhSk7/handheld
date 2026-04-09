#include "launcher/core/sdl_manager.hpp"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <array>

#include "launcher/core/constants.hpp"

namespace launcher {

namespace {

std::string sdlError(const std::string& message) {
    return message + ": " + SDL_GetError();
}

}  // namespace

SDL2Manager::SDL2Manager(int width, int height)
    : width_(width), height_(height) {
    init();
}

void SDL2Manager::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(sdlError("Failed to initialize SDL2"));
    }

    const int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP;
    if ((IMG_Init(imgFlags) & imgFlags) == 0) {
        throw std::runtime_error("Failed to initialize SDL2_image");
    }

    if (TTF_Init() < 0) {
        throw std::runtime_error("Failed to initialize SDL2_ttf");
    }

    window_ = SDL_CreateWindow(
        "Handheld Launcher",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width_,
        height_,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    if (!window_) {
        throw std::runtime_error(sdlError("Failed to create SDL window"));
    }

    SDL_ShowWindow(window_);

    const std::array<Uint32, 3> rendererFlags = {
        SDL_RENDERER_ACCELERATED,
        0,
        SDL_RENDERER_SOFTWARE,
    };

    for (const auto flags : rendererFlags) {
        renderer_ = SDL_CreateRenderer(window_, -1, flags);
        if (renderer_) {
            break;
        }
    }

    if (!renderer_) {
        throw std::runtime_error(sdlError("Failed to create SDL renderer"));
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer_, width_, height_);
}

SDL2Manager::~SDL2Manager() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

SDL_Window* SDL2Manager::window() const {
    return window_;
}

SDL_Renderer* SDL2Manager::renderer() const {
    return renderer_;
}

Renderer::Renderer(SDL2Manager& manager)
    : manager_(manager), renderer_(manager.renderer()) {}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer_, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, COLOR_BG.a);
    SDL_RenderClear(renderer_);
}

void Renderer::present() {
    SDL_RenderPresent(renderer_);
}

void Renderer::drawRect(int x, int y, int width, int height, const SDL_Color& color, bool filled) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_Rect rect{x, y, width, height};
    if (filled) {
        SDL_RenderFillRect(renderer_, &rect);
    } else {
        SDL_RenderDrawRect(renderer_, &rect);
    }
}

void Renderer::drawTexture(SDL_Texture* texture, int x, int y, int width, int height, bool cropToSquare) {
    if (!texture) {
        return;
    }

    SDL_Rect dstRect{x, y, width, height};
    SDL_Rect* srcRectPtr = nullptr;
    SDL_Rect srcRect{};

    if (cropToSquare) {
        int texW = 0;
        int texH = 0;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH) == 0) {
            const int cropSide = texW < texH ? texW : texH;
            srcRect.x = (texW - cropSide) / 2;
            srcRect.y = (texH - cropSide) / 2;
            srcRect.w = cropSide;
            srcRect.h = cropSide;
            srcRectPtr = &srcRect;
        }
    }

    SDL_RenderCopy(renderer_, texture, srcRectPtr, &dstRect);
}

SDL_Renderer* Renderer::raw() const {
    return renderer_;
}

}  // namespace launcher
