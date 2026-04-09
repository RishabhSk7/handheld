#pragma once

#include <SDL2/SDL.h>

#include <string_view>
#include <vector>

namespace launcher {

inline constexpr int SCREEN_WIDTH = 320;
inline constexpr int SCREEN_HEIGHT = 240;
inline constexpr int TARGET_FPS = 60;
inline constexpr int FRAME_TIME_MS = 1000 / TARGET_FPS;

inline constexpr SDL_Color COLOR_BG{10, 7, 24, 255};
inline constexpr SDL_Color COLOR_PANEL{20, 12, 45, 255};
inline constexpr SDL_Color COLOR_PANEL_ALT{13, 9, 32, 255};

inline constexpr SDL_Color COLOR_NEON_PURPLE{180, 74, 255, 255};
inline constexpr SDL_Color COLOR_NEON_PINK{255, 56, 190, 255};
inline constexpr SDL_Color COLOR_NEON_CYAN{84, 220, 255, 255};

inline constexpr SDL_Color COLOR_TEXT{220, 206, 255, 255};
inline constexpr SDL_Color COLOR_TEXT_STRONG{244, 236, 255, 255};
inline constexpr SDL_Color COLOR_TEXT_MUTED{152, 137, 199, 255};

inline constexpr SDL_Color COLOR_HIGHLIGHT = COLOR_NEON_PURPLE;
inline constexpr SDL_Color COLOR_DIM = COLOR_TEXT_MUTED;
inline constexpr SDL_Color COLOR_BORDER_GLOW = COLOR_NEON_PINK;

inline constexpr int UI_MARGIN = 12;
inline constexpr int UI_TOP_BAR_H = 28;
inline constexpr int UI_BOTTOM_BAR_H = 26;

inline const std::vector<std::string_view> VALID_ROM_EXTENSIONS = {
    ".nes", ".sfc", ".smc", ".gba", ".gb", ".gbc", ".zip",
};

}  // namespace launcher
