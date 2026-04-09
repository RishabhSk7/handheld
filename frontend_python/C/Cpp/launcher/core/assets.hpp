#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace launcher {

class Renderer;
class SDL2Manager;

struct TextureAsset {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
};

class AssetManager {
public:
    AssetManager(SDL2Manager& sdl2Manager, Renderer& renderer, std::filesystem::path assetsDir);
    ~AssetManager();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    TextureAsset* logoSprite() const;
    TextureAsset* getSystemIconSprite(const std::optional<std::string>& system);
    TextureAsset* getRomIconSprite(const std::optional<std::string>& system, const std::optional<std::string>& romFilename);
    TextureAsset* getRomPreviewSprite(const std::optional<std::string>& system, const std::optional<std::string>& romFilename);

    TextureAsset* renderText(const std::string& text, SDL_Color color, int size = 16);
    void drawTextOrPlaceholder(
        Renderer& renderer,
        const std::string& text,
        int x,
        int y,
        SDL_Color color,
        int size,
        int placeholderW,
        int placeholderH
    );
    void drawTextShadowed(
        Renderer& renderer,
        const std::string& text,
        int x,
        int y,
        SDL_Color fgColor,
        std::optional<SDL_Color> shadowColor = std::nullopt,
        int size = 16,
        int offset = 1
    );

    static std::string truncateText(const std::string& text, int maxLen = 20);

private:
    SDL2Manager& sdl2Manager_;
    Renderer& renderer_;
    std::filesystem::path assetsDir_;
    SDL_Renderer* rawRenderer_;

    std::optional<std::filesystem::path> activeFontPath_;
    std::unordered_map<int, TTF_Font*> fontsBySize_;

    std::unordered_map<std::string, std::unique_ptr<TextureAsset>> textCache_;
    std::unordered_map<std::string, std::unique_ptr<TextureAsset>> imageCache_;

    TextureAsset* logoSprite_ = nullptr;
    TextureAsset* romPlaceholderSprite_ = nullptr;
    TextureAsset* gamePlaceholderSprite_ = nullptr;
    TextureAsset* systemPlaceholderSprite_ = nullptr;

    void discoverFont();
    TTF_Font* getFont(int size);
    TextureAsset* loadImage(const std::filesystem::path& path);
    TextureAsset* loadFirstImage(const std::vector<std::filesystem::path>& candidates);
    std::vector<std::filesystem::path> romArtCandidates(const std::string& system, const std::string& romFilename, const std::string& artType) const;
    std::vector<std::filesystem::path> systemIconCandidates(const std::string& system) const;
    std::string textCacheKey(const std::string& text, SDL_Color color, int size) const;
};

}  // namespace launcher
