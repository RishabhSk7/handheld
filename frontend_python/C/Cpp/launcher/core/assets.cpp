#include "launcher/core/assets.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include "launcher/core/constants.hpp"
#include "launcher/core/sdl_manager.hpp"

namespace launcher {

namespace {

bool pathExists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

}  // namespace

AssetManager::AssetManager(SDL2Manager& sdl2Manager, Renderer& renderer, std::filesystem::path assetsDir)
    : sdl2Manager_(sdl2Manager),
      renderer_(renderer),
      assetsDir_(std::move(assetsDir)),
      rawRenderer_(renderer.raw()) {
    std::filesystem::create_directories(assetsDir_);

    discoverFont();

    logoSprite_ = loadFirstImage({
        std::filesystem::path("./resources/logo.png"),
        std::filesystem::path("../resources/logo.png"),
        std::filesystem::path("../../resources/logo.png"),
        assetsDir_ / "logo.png",
        assetsDir_ / "logo.bmp",
    });

    romPlaceholderSprite_ = loadFirstImage({
        std::filesystem::path("./resources/rom_placeholder.jpg"),
        std::filesystem::path("./resources/rom_placeholder.png"),
        assetsDir_ / "rom_placeholder.jpg",
        assetsDir_ / "rom_placeholder.png",
    });

    gamePlaceholderSprite_ = loadFirstImage({
        std::filesystem::path("./resources/game_placeholder.jpg"),
        std::filesystem::path("./resources/game_placeholder.png"),
        std::filesystem::path("./resources/games_placeholder.jpg"),
        std::filesystem::path("./resources/games_placeholder.png"),
        assetsDir_ / "game_placeholder.jpg",
        assetsDir_ / "game_placeholder.png",
        assetsDir_ / "games_placeholder.jpg",
        assetsDir_ / "games_placeholder.png",
    });

    systemPlaceholderSprite_ = loadFirstImage({
        std::filesystem::path("./resources/system_placeholder.jpg"),
        std::filesystem::path("./resources/system_placeholder.png"),
        assetsDir_ / "system_placeholder.jpg",
        assetsDir_ / "system_placeholder.png",
    });
}

AssetManager::~AssetManager() {
    for (auto& [_, entry] : textCache_) {
        if (entry && entry->texture) {
            SDL_DestroyTexture(entry->texture);
            entry->texture = nullptr;
        }
    }
    for (auto& [_, entry] : imageCache_) {
        if (entry && entry->texture) {
            SDL_DestroyTexture(entry->texture);
            entry->texture = nullptr;
        }
    }
    for (auto& [_, font] : fontsBySize_) {
        if (font) {
            TTF_CloseFont(font);
        }
    }
}

void AssetManager::discoverFont() {
    const std::vector<std::filesystem::path> candidates = {
        assetsDir_ / "font.ttf",
        assetsDir_ / "fonts" / "DejaVuSans.ttf",
        assetsDir_ / "fonts" / "Roboto-Regular.ttf",
        assetsDir_ / "fonts" / "LiberationSans-Regular.ttf",
        std::filesystem::path("./resources/fonts/DejaVuSans.ttf"),
        std::filesystem::path("./resources/fonts/Roboto-Regular.ttf"),
        std::filesystem::path("./resources/fonts/LiberationSans-Regular.ttf"),
        std::filesystem::path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
        std::filesystem::path("/usr/share/fonts/dejavu/DejaVuSans.ttf"),
        std::filesystem::path("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"),
        std::filesystem::path("/usr/share/fonts/truetype/freefont/FreeSans.ttf"),
        std::filesystem::path("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"),
    };

    for (const auto& path : candidates) {
        if (pathExists(path)) {
            activeFontPath_ = path;
            std::cout << "[INFO] Using font: " << path << '\n';
            return;
        }
    }

    const std::vector<std::filesystem::path> roots = {
        std::filesystem::path("/usr/share/fonts"),
        assetsDir_,
        std::filesystem::path("./resources"),
    };

    for (const auto& root : roots) {
        if (!pathExists(root)) {
            continue;
        }
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (ext == ".ttf") {
                activeFontPath_ = entry.path();
                std::cout << "[INFO] Using discovered font: " << entry.path() << '\n';
                return;
            }
        }
    }

    std::cout << "[WARNING] No TTF font found. Text rendering disabled.\n";
}

TTF_Font* AssetManager::getFont(int size) {
    if (!activeFontPath_.has_value()) {
        return nullptr;
    }

    const auto it = fontsBySize_.find(size);
    if (it != fontsBySize_.end()) {
        return it->second;
    }

    auto* font = TTF_OpenFont(activeFontPath_->string().c_str(), size);
    if (!font) {
        return nullptr;
    }

    fontsBySize_[size] = font;
    return font;
}

TextureAsset* AssetManager::loadImage(const std::filesystem::path& path) {
    const auto absolute = std::filesystem::absolute(path).string();

    const auto cacheIt = imageCache_.find(absolute);
    if (cacheIt != imageCache_.end()) {
        return cacheIt->second.get();
    }

    if (!pathExists(path)) {
        imageCache_[absolute] = nullptr;
        return nullptr;
    }

    SDL_Texture* texture = IMG_LoadTexture(rawRenderer_, path.string().c_str());
    if (!texture) {
        std::cout << "[WARNING] Could not load image " << path << '\n';
        imageCache_[absolute] = nullptr;
        return nullptr;
    }

    int width = 0;
    int height = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    auto sprite = std::make_unique<TextureAsset>();
    sprite->texture = texture;
    sprite->width = width;
    sprite->height = height;

    auto* ptr = sprite.get();
    imageCache_[absolute] = std::move(sprite);
    return ptr;
}

TextureAsset* AssetManager::loadFirstImage(const std::vector<std::filesystem::path>& candidates) {
    for (const auto& candidate : candidates) {
        auto* sprite = loadImage(candidate);
        if (sprite) {
            return sprite;
        }
    }
    return nullptr;
}

std::vector<std::filesystem::path> AssetManager::romArtCandidates(
    const std::string& system,
    const std::string& romFilename,
    const std::string& artType
) const {
    const auto romPath = std::filesystem::path(romFilename);
    const auto romStem = romPath.stem().string();
    const auto romFull = romPath.filename().string();
    const std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".webp", ".bmp"};

    const std::vector<std::filesystem::path> roots = {
        assetsDir_ / artType / system,
        assetsDir_ / "boxart" / system,
        std::filesystem::path("./resources") / artType / system,
        std::filesystem::path("./resources") / "boxart" / system,
    };

    std::vector<std::filesystem::path> candidates;
    for (const auto& root : roots) {
        for (const auto& ext : extensions) {
            candidates.push_back(root / (romStem + ext));
            candidates.push_back(root / (romFull + ext));
        }
    }
    return candidates;
}

std::vector<std::filesystem::path> AssetManager::systemIconCandidates(const std::string& system) const {
    std::string lower = system;
    std::string upper = system;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });

    const std::vector<std::string> names = {system, lower, upper};
    const std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".webp", ".bmp"};
    const std::vector<std::filesystem::path> roots = {
        assetsDir_ / "systems",
        assetsDir_ / "icons" / "systems",
        assetsDir_ / "icons",
        std::filesystem::path("./resources") / "systems",
        std::filesystem::path("./resources") / "icons" / "systems",
        std::filesystem::path("./resources") / "icons",
    };

    std::vector<std::filesystem::path> candidates;
    for (const auto& root : roots) {
        for (const auto& name : names) {
            for (const auto& ext : extensions) {
                candidates.push_back(root / (name + ext));
            }
        }
    }
    return candidates;
}

std::string AssetManager::textCacheKey(const std::string& text, SDL_Color color, int size) const {
    std::ostringstream out;
    out << text << "|" << size << "|"
        << static_cast<int>(color.r) << ","
        << static_cast<int>(color.g) << ","
        << static_cast<int>(color.b) << ","
        << static_cast<int>(color.a);
    return out.str();
}

TextureAsset* AssetManager::logoSprite() const {
    return logoSprite_;
}

TextureAsset* AssetManager::getSystemIconSprite(const std::optional<std::string>& system) {
    if (system.has_value()) {
        auto* sprite = loadFirstImage(systemIconCandidates(*system));
        if (sprite) {
            return sprite;
        }
    }
    return systemPlaceholderSprite_;
}

TextureAsset* AssetManager::getRomIconSprite(const std::optional<std::string>& system, const std::optional<std::string>& romFilename) {
    if (system.has_value() && romFilename.has_value()) {
        auto* sprite = loadFirstImage(romArtCandidates(*system, *romFilename, "icons"));
        if (sprite) {
            return sprite;
        }
    }
    return romPlaceholderSprite_;
}

TextureAsset* AssetManager::getRomPreviewSprite(const std::optional<std::string>& system, const std::optional<std::string>& romFilename) {
    if (system.has_value() && romFilename.has_value()) {
        auto* sprite = loadFirstImage(romArtCandidates(*system, *romFilename, "boxart"));
        if (sprite) {
            return sprite;
        }
    }
    return gamePlaceholderSprite_;
}

TextureAsset* AssetManager::renderText(const std::string& text, SDL_Color color, int size) {
    const auto key = textCacheKey(text, color, size);
    const auto it = textCache_.find(key);
    if (it != textCache_.end()) {
        return it->second.get();
    }

    auto* font = getFont(size);
    if (!font) {
        return nullptr;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(rawRenderer_, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return nullptr;
    }

    auto sprite = std::make_unique<TextureAsset>();
    sprite->texture = texture;
    sprite->width = surface->w;
    sprite->height = surface->h;
    SDL_FreeSurface(surface);

    auto* ptr = sprite.get();
    textCache_[key] = std::move(sprite);
    return ptr;
}

void AssetManager::drawTextOrPlaceholder(
    Renderer& renderer,
    const std::string& text,
    int x,
    int y,
    SDL_Color color,
    int size,
    int placeholderW,
    int placeholderH
) {
    auto* sprite = renderText(text, color, size);
    if (sprite) {
        renderer.drawTexture(sprite->texture, x, y, sprite->width, sprite->height);
        return;
    }
    renderer.drawRect(x, y, placeholderW, placeholderH, color, false);
}

void AssetManager::drawTextShadowed(
    Renderer& renderer,
    const std::string& text,
    int x,
    int y,
    SDL_Color fgColor,
    std::optional<SDL_Color> shadowColor,
    int size,
    int offset
) {
    const SDL_Color shadow = shadowColor.value_or(SDL_Color{12, 8, 28, 255});

    auto* shadowSprite = renderText(text, shadow, size);
    if (shadowSprite) {
        renderer.drawTexture(shadowSprite->texture, x + offset, y + offset, shadowSprite->width, shadowSprite->height);
    }

    const int placeholderW = std::max(48, static_cast<int>(text.size()) * 6);
    const int placeholderH = std::max(10, size / 2);
    drawTextOrPlaceholder(renderer, text, x, y, fgColor, size, placeholderW, placeholderH);
}

std::string AssetManager::truncateText(const std::string& text, int maxLen) {
    if (static_cast<int>(text.size()) <= maxLen) {
        return text;
    }
    return text.substr(0, static_cast<std::size_t>(maxLen)) + "...";
}

}  // namespace launcher
