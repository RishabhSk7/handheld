#include "launcher/ui/splash.hpp"

#include <algorithm>
#include <cmath>

#include "launcher/core/constants.hpp"

namespace launcher {

SplashScreen::SplashScreen(double durationSeconds)
    : durationSeconds_(durationSeconds) {}

void SplashScreen::start() {
    startTicks_ = SDL_GetTicks();
    started_ = true;
}

bool SplashScreen::update() {
    if (!started_) {
        return false;
    }

    const double elapsed = static_cast<double>(SDL_GetTicks() - startTicks_) / 1000.0;
    return elapsed >= durationSeconds_;
}

void SplashScreen::render(Renderer& renderer, AssetManager& assets) {
    const double t = static_cast<double>(SDL_GetTicks()) / 1000.0;
    const double pulse = 0.5 + (0.5 * std::sin(t * 3.2));

    const int panelX = 42;
    const int panelY = 56;
    const int panelW = SCREEN_WIDTH - 84;
    const int panelH = 128;
    renderer.drawRect(panelX, panelY, panelW, panelH, COLOR_PANEL, true);

    const int glowInset = pulse > 0.5 ? 2 : 3;
    renderer.drawRect(panelX - glowInset, panelY - glowInset, panelW + (glowInset * 2), panelH + (glowInset * 2), COLOR_BORDER_GLOW, false);
    renderer.drawRect(panelX + 3, panelY + 3, panelW - 6, panelH - 6, COLOR_NEON_CYAN, false);

    if (auto* logo = assets.logoSprite()) {
        const int maxLogoW = panelW - 24;
        const int maxLogoH = panelH - 32;
        const double scale = std::min({
            static_cast<double>(maxLogoW) / static_cast<double>(logo->width),
            static_cast<double>(maxLogoH) / static_cast<double>(logo->height),
            1.0,
        });

        const int drawW = std::max(1, static_cast<int>(logo->width * scale));
        const int drawH = std::max(1, static_cast<int>(logo->height * scale));
        const int x = panelX + (panelW - drawW) / 2;
        const int y = panelY + (panelH - drawH) / 2;
        renderer.drawTexture(logo->texture, x, y, drawW, drawH);
    } else {
        renderer.drawRect(96, 88, 128, 52, COLOR_DIM, false);
    }

    auto* titleText = assets.renderText("HANDHELD LAUNCHER", COLOR_TEXT, 16);
    if (titleText) {
        const int x = (SCREEN_WIDTH - titleText->width) / 2;
        const int y = 192;
        assets.drawTextShadowed(renderer, "HANDHELD LAUNCHER", x, y, COLOR_TEXT, COLOR_PANEL, 16, 1);

        if (pulse > 0.5) {
            renderer.drawRect(x - 4, y + 16, titleText->width + 8, 1, COLOR_HIGHLIGHT, true);
        }
    } else {
        renderer.drawRect(88, 192, 144, 14, COLOR_TEXT, false);
    }

    assets.drawTextShadowed(renderer, "LOADING RETRO CORE...", 94, 213, COLOR_TEXT_MUTED, COLOR_PANEL, 10, 1);
}

}  // namespace launcher
