import math
import time
from typing import List, Optional, Tuple

from launcher.core.assets import AssetManager
from launcher.core.constants import (
    COLOR_BORDER_GLOW,
    COLOR_DIM,
    COLOR_HIGHLIGHT,
    COLOR_NEON_CYAN,
    COLOR_PANEL,
    COLOR_PANEL_ALT,
    COLOR_TEXT,
    SCREEN_HEIGHT,
)
from launcher.core.library import GameLibrary
from launcher.core.sdl_manager import Renderer


class MainMenu:
    """Horizontal carousel menu for selecting systems with smooth animation."""

    def __init__(self, game_library: GameLibrary):
        self.systems = game_library.get_systems()
        self.current_index = 0
        self.target_index = 0
        self.offset = 0.0
        self.smoothing = 8.0
        self.items_visible = 3

    def move_left(self):
        """Move selection left in carousel."""
        if self.systems:
            self.target_index = (self.target_index - 1) % len(self.systems)

    def move_right(self):
        """Move selection right in carousel."""
        if self.systems:
            self.target_index = (self.target_index + 1) % len(self.systems)

    def update(self, delta_time: float):
        """Update animation with time-based interpolation toward target."""
        if not self.systems:
            return

        count = len(self.systems)
        raw_diff = self.target_index - self.current_index
        if raw_diff > count / 2:
            raw_diff -= count
        elif raw_diff < -count / 2:
            raw_diff += count

        alpha = min(1.0, self.smoothing * delta_time)
        self.offset += (raw_diff - self.offset) * alpha

        if abs(raw_diff - self.offset) < 0.01:
            self.current_index = self.target_index
            self.offset = 0.0

    def get_selected_system(self) -> Optional[str]:
        """Get currently selected system."""
        if self.systems:
            selected_offset = int(round(self.offset))
            selected_index = (self.current_index + selected_offset) % len(self.systems)
            return self.systems[selected_index]
        return None

    def get_visible_systems(self) -> List[Tuple[str, bool, float]]:
        """Get visible systems in carousel with animated offset."""
        if not self.systems:
            return []

        count = len(self.systems)
        position = self.current_index + self.offset
        items = []

        for idx, system in enumerate(self.systems):
            diff = idx - position
            if diff > count / 2:
                diff -= count
            elif diff < -count / 2:
                diff += count

            if abs(diff) <= 1.6:
                items.append((system, False, diff))

        if not items:
            return []

        selected_system = min(items, key=lambda item: abs(item[2]))[0]
        return [(system, system == selected_system, effective_offset) for system, _, effective_offset in items]

    def render(self, renderer: Renderer, assets: AssetManager, game_library: GameLibrary):
        """Render main menu carousel."""
        visible = self.get_visible_systems()
        pulse = 0.5 + (0.5 * math.sin(time.time() * 4.2))

        center_x = 160
        item_spacing = 80
        selected_frame_height = 86
        frame_top = (SCREEN_HEIGHT - selected_frame_height) // 2
        base_icon_y = frame_top + 18
        base_text_y = base_icon_y + 44

        renderer.draw_rect(20, frame_top - 10, 280, selected_frame_height + 20, COLOR_PANEL, filled=True)
        renderer.draw_rect(20, frame_top - 10, 280, selected_frame_height + 20, COLOR_NEON_CYAN, filled=False)
        renderer.draw_rect(24, frame_top - 6, 272, selected_frame_height + 12, COLOR_PANEL_ALT, filled=False)

        for system, is_selected, effective_offset in sorted(visible, key=lambda item: abs(item[2]), reverse=True):
            x = center_x + int(effective_offset * item_spacing)
            icon_y = base_icon_y
            text_y = base_text_y

            if x < 30 or x > 290:
                continue

            icon_size = 40 if is_selected else 26
            icon_x = x - (icon_size // 2)
            icon_sprite = assets.get_system_icon_sprite(system)

            if is_selected:
                renderer.draw_rect(icon_x - 5, icon_y - 5, icon_size + 10, icon_size + 10, COLOR_BORDER_GLOW, filled=False)
                if pulse > 0.5:
                    renderer.draw_rect(icon_x - 3, icon_y - 3, icon_size + 6, icon_size + 6, COLOR_HIGHLIGHT, filled=False)

            if icon_sprite:
                renderer.draw_texture(
                    icon_sprite.texture,
                    icon_x,
                    icon_y,
                    icon_size,
                    icon_size,
                    crop_to_square=True,
                )
            else:
                renderer.draw_rect(icon_x, icon_y, icon_size, icon_size, COLOR_DIM, filled=False)

            display_name = game_library.get_system_display_name(system)
            display_name = AssetManager.truncate_text(display_name, max_len=10)

            color = COLOR_HIGHLIGHT if is_selected else COLOR_TEXT
            size = 18 if is_selected else 13

            text_sprite = assets.render_text(display_name, color, size)
            if text_sprite:
                text_x = x - (text_sprite.size[0] // 2)
                assets.draw_text_shadowed(renderer, display_name, text_x, text_y, color, COLOR_PANEL_ALT, size, 1)
            else:
                renderer.draw_rect(x - 30, text_y + 2, 60, 12, color, filled=False)

            if is_selected:
                renderer.draw_rect(x - 54, frame_top + 2, 108, selected_frame_height - 8, COLOR_HIGHLIGHT, filled=False)
