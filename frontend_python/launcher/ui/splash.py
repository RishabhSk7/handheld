import time
import math

from launcher.core.assets import AssetManager
from launcher.core.constants import (
    COLOR_BORDER_GLOW,
    COLOR_DIM,
    COLOR_HIGHLIGHT,
    COLOR_NEON_CYAN,
    COLOR_PANEL,
    COLOR_TEXT,
    COLOR_TEXT_MUTED,
    SCREEN_HEIGHT,
    SCREEN_WIDTH,
)
from launcher.core.sdl_manager import Renderer


class SplashScreen:
    """Splash screen with logo and delay."""

    def __init__(self, duration_seconds: float = 2.0):
        self.duration = duration_seconds
        self.start_time = None
        self.finished = False

    def start(self):
        """Start the splash screen timer."""
        self.start_time = time.time()
        self.finished = False

    def update(self):
        """Update splash state. Returns True if finished."""
        if self.start_time is None:
            return False

        elapsed = time.time() - self.start_time
        if elapsed >= self.duration:
            self.finished = True
            return True

        return False

    def render(self, renderer: Renderer, assets: AssetManager):
        """Render splash screen."""
        pulse = 0.5 + (0.5 * math.sin(time.time() * 3.2))

        panel_x = 42
        panel_y = 56
        panel_w = SCREEN_WIDTH - 84
        panel_h = 128
        renderer.draw_rect(panel_x, panel_y, panel_w, panel_h, COLOR_PANEL, filled=True)

        glow_inset = 2 if pulse > 0.5 else 3
        renderer.draw_rect(panel_x - glow_inset, panel_y - glow_inset, panel_w + (glow_inset * 2), panel_h + (glow_inset * 2), COLOR_BORDER_GLOW, filled=False)
        renderer.draw_rect(panel_x + 3, panel_y + 3, panel_w - 6, panel_h - 6, COLOR_NEON_CYAN, filled=False)

        if assets.logo_sprite:
            logo_w, logo_h = assets.logo_sprite.size
            max_logo_w = panel_w - 24
            max_logo_h = panel_h - 32
            scale = min(max_logo_w / logo_w, max_logo_h / logo_h, 1.0)

            draw_w = max(1, int(logo_w * scale))
            draw_h = max(1, int(logo_h * scale))
            x = panel_x + (panel_w - draw_w) // 2
            y = panel_y + (panel_h - draw_h) // 2

            renderer.draw_texture(
                assets.logo_sprite.texture,
                x,
                y,
                draw_w,
                draw_h,
            )
        else:
            renderer.draw_rect(96, 88, 128, 52, COLOR_DIM, filled=False)

        title_text = assets.render_text("HANDHELD LAUNCHER", COLOR_TEXT, 16)
        if title_text:
            x = (SCREEN_WIDTH - title_text.size[0]) // 2
            y = 192
            assets.draw_text_shadowed(renderer, "HANDHELD LAUNCHER", x, y, COLOR_TEXT, COLOR_PANEL, 16, 1)

            if pulse > 0.5:
                renderer.draw_rect(x - 4, y + 16, title_text.size[0] + 8, 1, COLOR_HIGHLIGHT, filled=True)
        else:
            renderer.draw_rect(88, 192, 144, 14, COLOR_TEXT, filled=False)

        assets.draw_text_shadowed(
            renderer,
            "LOADING RETRO CORE...",
            94,
            213,
            COLOR_TEXT_MUTED,
            COLOR_PANEL,
            10,
            1,
        )
