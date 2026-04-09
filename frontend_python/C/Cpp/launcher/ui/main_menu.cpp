#include "launcher/ui/main_menu.hpp"

#include <algorithm>
#include <cmath>

#include "launcher/core/constants.hpp"

namespace launcher {

MainMenu::MainMenu(const GameLibrary& gameLibrary)
    : systems_(gameLibrary.getSystems()) {}

void MainMenu::moveLeft() {
    if (systems_.empty()) {
        return;
    }
    targetIndex_ = (targetIndex_ - 1 + static_cast<int>(systems_.size())) % static_cast<int>(systems_.size());
}

void MainMenu::moveRight() {
    if (systems_.empty()) {
        return;
    }
    targetIndex_ = (targetIndex_ + 1) % static_cast<int>(systems_.size());
}

void MainMenu::update(double deltaTime) {
    if (systems_.empty()) {
        return;
    }

    const int count = static_cast<int>(systems_.size());
    double rawDiff = static_cast<double>(targetIndex_ - currentIndex_);
    if (rawDiff > count / 2.0) {
        rawDiff -= count;
    } else if (rawDiff < -count / 2.0) {
        rawDiff += count;
    }

    const double alpha = std::min(1.0, smoothing_ * deltaTime);
    offset_ += (rawDiff - offset_) * alpha;

    if (std::abs(rawDiff - offset_) < 0.01) {
        currentIndex_ = targetIndex_;
        offset_ = 0.0;
    }
}

std::optional<std::string> MainMenu::getSelectedSystem() const {
    if (systems_.empty()) {
        return std::nullopt;
    }
    const int selectedOffset = static_cast<int>(std::round(offset_));
    const int selectedIndex = (currentIndex_ + selectedOffset + static_cast<int>(systems_.size())) % static_cast<int>(systems_.size());
    return systems_[selectedIndex];
}

std::vector<std::tuple<std::string, bool, double>> MainMenu::getVisibleSystems() const {
    std::vector<std::tuple<std::string, bool, double>> items;
    if (systems_.empty()) {
        return items;
    }

    const int count = static_cast<int>(systems_.size());
    const double position = static_cast<double>(currentIndex_) + offset_;

    std::vector<std::tuple<std::string, bool, double>> visible;
    for (int idx = 0; idx < count; ++idx) {
        double diff = static_cast<double>(idx) - position;
        if (diff > count / 2.0) {
            diff -= count;
        } else if (diff < -count / 2.0) {
            diff += count;
        }

        if (std::abs(diff) <= 1.6) {
            visible.emplace_back(systems_[idx], false, diff);
        }
    }

    if (visible.empty()) {
        return visible;
    }

    auto selectedIt = std::min_element(visible.begin(), visible.end(), [](const auto& a, const auto& b) {
        return std::abs(std::get<2>(a)) < std::abs(std::get<2>(b));
    });
    const auto selectedSystem = std::get<0>(*selectedIt);

    for (const auto& item : visible) {
        items.emplace_back(std::get<0>(item), std::get<0>(item) == selectedSystem, std::get<2>(item));
    }
    return items;
}

void MainMenu::render(Renderer& renderer, AssetManager& assets, const GameLibrary& gameLibrary) {
    const auto visible = getVisibleSystems();
    const double t = static_cast<double>(SDL_GetTicks()) / 1000.0;
    const double pulse = 0.5 + (0.5 * std::sin(t * 4.2));

    const int centerX = 160;
    const int itemSpacing = 80;
    const int selectedFrameHeight = 86;
    const int frameTop = (SCREEN_HEIGHT - selectedFrameHeight) / 2;
    const int baseIconY = frameTop + 18;
    const int baseTextY = baseIconY + 44;

    renderer.drawRect(20, frameTop - 10, 280, selectedFrameHeight + 20, COLOR_PANEL, false);
    renderer.drawRect(20, frameTop - 10, 280, selectedFrameHeight + 20, COLOR_NEON_CYAN, false);
    renderer.drawRect(24, frameTop - 6, 272, selectedFrameHeight + 12, COLOR_PANEL_ALT, false);

    auto sortedVisible = visible;
    std::sort(sortedVisible.begin(), sortedVisible.end(), [](const auto& a, const auto& b) {
        return std::abs(std::get<2>(a)) > std::abs(std::get<2>(b));
    });

    for (const auto& [system, isSelected, effectiveOffset] : sortedVisible) {
        const int x = centerX + static_cast<int>(effectiveOffset * itemSpacing);
        if (x < 30 || x > 290) {
            continue;
        }

        const int iconSize = isSelected ? 40 : 26;
        const int iconX = x - (iconSize / 2);
        const int cardW = isSelected ? 78 : 52;
        const int cardH = isSelected ? 72 : 48;
        const int cardX = x - (cardW / 2);
        const int cardY = baseIconY - (isSelected ? 8 : 4);

        if (isSelected) {
            SDL_Color cardFill = COLOR_PANEL;
            cardFill.a = pulse > 0.5 ? 140 : 110;
            SDL_Color cardBorder = COLOR_TEXT_STRONG;
            cardBorder.a = pulse > 0.5 ? 190 : 150;

            renderer.drawRect(cardX + 1, cardY, cardW - 2, cardH, cardFill, true);
            renderer.drawRect(cardX, cardY + 1, cardW, cardH - 2, cardFill, true);
            renderer.drawRect(cardX + 1, cardY, cardW - 2, cardH, cardBorder, false);
            renderer.drawRect(cardX, cardY + 1, cardW, cardH - 2, cardBorder, false);
        }

        auto* iconSprite = assets.getSystemIconSprite(system);

        if (iconSprite) {
            renderer.drawTexture(iconSprite->texture, iconX, baseIconY, iconSize, iconSize, true);
        } else {
            renderer.drawRect(iconX, baseIconY, iconSize, iconSize, COLOR_DIM, false);
        }

        const auto displayName = AssetManager::truncateText(gameLibrary.getSystemDisplayName(system), 10);
        SDL_Color color = isSelected ? COLOR_HIGHLIGHT : COLOR_TEXT;
        if (isSelected && pulse < 0.5) {
            color = COLOR_TEXT_STRONG;
        }
        const int size = isSelected ? 18 : 13;
        auto* textSprite = assets.renderText(displayName, color, size);

        if (textSprite) {
            const int textX = x - (textSprite->width / 2);
            assets.drawTextShadowed(renderer, displayName, textX, baseTextY, color, COLOR_PANEL_ALT, size, 1);
        } else {
            renderer.drawRect(x - 30, baseTextY + 2, 60, 12, color, false);
        }
    }
}

}  // namespace launcher
