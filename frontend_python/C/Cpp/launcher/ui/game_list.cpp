#include "launcher/ui/game_list.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

#include "launcher/core/constants.hpp"

namespace launcher {

GameList::GameList(const GameLibrary& gameLibrary)
    : gameLibrary_(gameLibrary) {}

void GameList::setSystem(const std::string& system) {
    currentSystem_ = system;
    roms_ = gameLibrary_.getRoms(system);
    currentIndex_ = 0;
    targetIndex_ = 0;
    offset_ = 0.0;
    std::cout << "[GameList] Loaded " << roms_.size() << " ROMs for system '" << system << "'\n";
}

void GameList::moveUp() {
    if (roms_.empty()) {
        return;
    }
    targetIndex_ = (targetIndex_ - 1 + static_cast<int>(roms_.size())) % static_cast<int>(roms_.size());
}

void GameList::moveDown() {
    if (roms_.empty()) {
        return;
    }
    targetIndex_ = (targetIndex_ + 1) % static_cast<int>(roms_.size());
}

void GameList::update(double deltaTime) {
    if (roms_.empty()) {
        return;
    }

    const int count = static_cast<int>(roms_.size());
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

std::optional<std::string> GameList::getSelectedRom() const {
    if (roms_.empty()) {
        return std::nullopt;
    }

    const int selectedOffset = static_cast<int>(std::round(offset_));
    const int selectedIndex = (currentIndex_ + selectedOffset + static_cast<int>(roms_.size())) % static_cast<int>(roms_.size());
    return roms_[selectedIndex];
}

std::vector<std::tuple<std::string, bool, double>> GameList::getVisibleRoms() const {
    std::vector<std::tuple<std::string, bool, double>> items;
    if (roms_.empty()) {
        return items;
    }

    const int count = static_cast<int>(roms_.size());
    const double animatedIndex = static_cast<double>(currentIndex_) + offset_;
    const int maxVisibleOffset = (itemsVisible_ / 2) + 1;

    for (int idx = 0; idx < count; ++idx) {
        double effectiveOffset = static_cast<double>(idx) - animatedIndex;
        if (effectiveOffset > count / 2.0) {
            effectiveOffset -= count;
        } else if (effectiveOffset < -count / 2.0) {
            effectiveOffset += count;
        }

        if (std::abs(effectiveOffset) > (maxVisibleOffset + 0.5)) {
            continue;
        }

        const bool isSelected = std::abs(effectiveOffset) < 0.5;
        items.emplace_back(roms_[idx], isSelected, effectiveOffset);
    }

    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        return std::get<2>(a) < std::get<2>(b);
    });
    return items;
}

void GameList::render(Renderer& renderer, AssetManager& assets) {
    const auto visible = getVisibleRoms();
    const double t = static_cast<double>(SDL_GetTicks()) / 1000.0;
    const double pulse = 0.5 + (0.5 * std::sin(t * 3.4));

    const int centerY = 120;
    const int lineHeight = 30;

    const int previewW = 104;
    const int previewH = 104;
    const int previewX = 320 - previewW - 12;
    const int previewY = (240 - previewH) / 2;

    const int listX = 12;
    const int sectionGap = 8;
    const int listOuterLeft = listX - 2;
    const int previewOuterLeft = previewX - 8;
    const int listOuterRight = previewOuterLeft - sectionGap;
    const int listW = std::max(140, listOuterRight - listOuterLeft - 4);
    const int listInnerX = listX + 2;
    const int listInnerW = listW - 4;
    const int listCenterX = listInnerX + (listInnerW / 2);

    const int listPanelY = 38;
    const int listPanelH = 172;
    renderer.drawRect(listX - 2, listPanelY, listW + 4, listPanelH, COLOR_PANEL, false);
    renderer.drawRect(listX - 2, listPanelY, listW + 4, listPanelH, COLOR_NEON_CYAN, false);
    renderer.drawRect(listX + 2, listPanelY + 4, listW - 4, listPanelH - 8, COLOR_PANEL_ALT, false);

    renderer.drawRect(previewX - 8, previewY - 10, previewW + 16, previewH + 20, COLOR_PANEL, false);
    renderer.drawRect(previewX - 8, previewY - 10, previewW + 16, previewH + 20, COLOR_NEON_CYAN, false);
    renderer.drawRect(previewX - 3, previewY - 3, previewW + 6, previewH + 6, COLOR_BORDER_GLOW, false);

    const auto selectedRom = getSelectedRom();
    auto* previewSprite = assets.getRomPreviewSprite(currentSystem_, selectedRom);
    if (previewSprite) {
        renderer.drawTexture(previewSprite->texture, previewX, previewY, previewW, previewH, true);
    } else {
        renderer.drawRect(previewX, previewY, previewW, previewH, COLOR_TEXT, false);
    }

    for (const auto& [rom, isSelected, effectiveOffset] : visible) {
        const int rowY = centerY + static_cast<int>(effectiveOffset * lineHeight);
        if (rowY < 34 || rowY > 212) {
            continue;
        }

        const int rowTop = rowY - 14;

        SDL_Color textColor = isSelected ? COLOR_HIGHLIGHT : COLOR_TEXT_MUTED;
        if (isSelected && pulse < 0.5) {
            textColor = COLOR_TEXT_STRONG;
        }
        const int textSize = isSelected ? 16 : 14;
        const auto displayName = std::filesystem::path(rom).stem().string();
        const auto truncated = AssetManager::truncateText(displayName, 22);
        auto* textSprite = assets.renderText(truncated, textColor, textSize);

        if (isSelected) {
            SDL_Color selectionFill = COLOR_PANEL;
            selectionFill.a = pulse > 0.5 ? 135 : 105;
            SDL_Color selectionBorder = COLOR_TEXT_STRONG;
            selectionBorder.a = pulse > 0.5 ? 185 : 145;

            renderer.drawRect(listX + 5, rowTop + 4, listW - 10, 20, selectionFill, true);
            renderer.drawRect(listX + 4, rowTop + 5, listW - 8, 18, selectionFill, true);
            renderer.drawRect(listX + 5, rowTop + 4, listW - 10, 20, selectionBorder, false);
            renderer.drawRect(listX + 4, rowTop + 5, listW - 8, 18, selectionBorder, false);
        }

        if (textSprite) {
            const int centeredTextX = listCenterX - (textSprite->width / 2);
            const int centeredTextY = rowY - (textSprite->height / 2);
            assets.drawTextShadowed(renderer, truncated, centeredTextX, centeredTextY, textColor, COLOR_PANEL, textSize, 1);
        } else {
            renderer.drawRect(listInnerX + 6, rowTop + 10, listInnerW - 12, 10, textColor, false);
        }
    }
}

std::optional<std::string> GameList::currentSystem() const {
    return currentSystem_;
}

}  // namespace launcher
