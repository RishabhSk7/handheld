import math
import time
from pathlib import Path
from typing import List, Optional, Tuple

from launcher.core.assets import AssetManager
from launcher.core.constants import (
    COLOR_BORDER_GLOW,
    COLOR_HIGHLIGHT,
    COLOR_NEON_CYAN,
    COLOR_PANEL,
    COLOR_PANEL_ALT,
    COLOR_TEXT,
    COLOR_TEXT_MUTED,
)
from launcher.core.library import GameLibrary
from launcher.core.sdl_manager import Renderer


class GameList:
    """Vertical list of ROMs for a selected system with smooth scrolling."""

    def __init__(self, game_library: GameLibrary):
        self.game_library = game_library
        self.current_system = None
        self.roms = []
        self.current_index = 0
        self.target_index = 0
        self.offset = 0.0
        self.smoothing = 8.0
        self.items_visible = 5

    def set_system(self, system: str):
        """Set the system and load its ROMs."""
        self.current_system = system
        self.roms = self.game_library.get_roms(system)
        self.current_index = 0
        self.target_index = 0
        self.offset = 0.0
        print(f"[GameList] Loaded {len(self.roms)} ROMs for system '{system}'")

    def move_up(self):
        """Move selection up in list."""
        if self.roms:
            self.target_index = (self.target_index - 1) % len(self.roms)

    def move_down(self):
        """Move selection down in list."""
        if self.roms:
            self.target_index = (self.target_index + 1) % len(self.roms)

    def update(self, delta_time: float):
        """Update animation with time-based interpolation toward target."""
        if not self.roms:
            return

        count = len(self.roms)
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

    def get_selected_rom(self) -> Optional[str]:
        """Get currently selected ROM filename."""
        if self.roms:
            selected_offset = int(round(self.offset))
            selected_index = (self.current_index + selected_offset) % len(self.roms)
            return self.roms[selected_index]
        return None

    def get_visible_roms(self) -> List[Tuple[str, bool, float]]:
        """Get visible ROMs in list with animated offset."""
        if not self.roms:
            return []

        count = len(self.roms)
        animated_index = self.current_index + self.offset
        max_visible_offset = (self.items_visible // 2) + 1

        items = []
        for idx, rom_name in enumerate(self.roms):
            effective_offset = idx - animated_index

            if effective_offset > count / 2:
                effective_offset -= count
            elif effective_offset < -count / 2:
                effective_offset += count

            if abs(effective_offset) > (max_visible_offset + 0.5):
                continue

            is_selected = abs(effective_offset) < 0.5
            items.append((rom_name, is_selected, effective_offset))

        items.sort(key=lambda item: item[2])
        return items

    def render(self, renderer: Renderer, assets: AssetManager):
        """Render game list."""
        visible = self.get_visible_roms()
        pulse = 0.5 + (0.5 * math.sin(time.time() * 3.4))

        center_y = 120
        line_height = 30

        preview_w = 104
        preview_h = 104
        preview_x = 320 - preview_w - 12
        preview_y = (240 - preview_h) // 2

        list_x = 12
        section_gap = 8
        list_outer_left = list_x - 2
        preview_outer_left = preview_x - 8
        list_outer_right = preview_outer_left - section_gap
        list_w = max(140, list_outer_right - list_outer_left - 4)
        text_x = list_x + 8

        list_panel_y = 38
        list_panel_h = 172
        renderer.draw_rect(list_x - 2, list_panel_y, list_w + 4, list_panel_h, COLOR_PANEL, filled=True)
        renderer.draw_rect(list_x - 2, list_panel_y, list_w + 4, list_panel_h, COLOR_NEON_CYAN, filled=False)
        renderer.draw_rect(list_x + 2, list_panel_y + 4, list_w - 4, list_panel_h - 8, COLOR_PANEL_ALT, filled=False)

        renderer.draw_rect(preview_x - 8, preview_y - 10, preview_w + 16, preview_h + 20, COLOR_PANEL, filled=True)
        renderer.draw_rect(preview_x - 8, preview_y - 10, preview_w + 16, preview_h + 20, COLOR_NEON_CYAN, filled=False)
        renderer.draw_rect(preview_x - 3, preview_y - 3, preview_w + 6, preview_h + 6, COLOR_BORDER_GLOW, filled=False)
        if pulse > 0.5:
            renderer.draw_rect(preview_x - 1, preview_y - 1, preview_w + 2, preview_h + 2, COLOR_HIGHLIGHT, filled=False)

        selected_rom = self.get_selected_rom()
        preview_sprite = assets.get_rom_preview_sprite(self.current_system, selected_rom)
        if preview_sprite:
            renderer.draw_texture(
                preview_sprite.texture,
                preview_x,
                preview_y,
                preview_w,
                preview_h,
                crop_to_square=True,
            )
        else:
            renderer.draw_rect(preview_x, preview_y, preview_w, preview_h, COLOR_TEXT, filled=False)

        for rom, is_selected, effective_offset in visible:
            row_y = center_y + int(effective_offset * line_height)
            if row_y < 34 or row_y > 212:
                continue

            row_top = row_y - 14

            color = COLOR_HIGHLIGHT if is_selected else COLOR_TEXT
            display_name = Path(rom).stem
            truncated_rom = AssetManager.truncate_text(display_name, max_len=22)
            text_color = color if is_selected else COLOR_TEXT_MUTED
            text_sprite = assets.render_text(truncated_rom, text_color, 14)

            if is_selected:
                renderer.draw_rect(list_x + 1, row_top + 3, list_w - 2, 22, COLOR_PANEL_ALT, filled=True)
                renderer.draw_rect(list_x, row_top + 2, list_w, 24, COLOR_HIGHLIGHT, filled=False)
                if pulse > 0.55:
                    renderer.draw_rect(list_x + 2, row_top + 4, list_w - 4, 20, COLOR_BORDER_GLOW, filled=False)

            if text_sprite:
                assets.draw_text_shadowed(renderer, truncated_rom, text_x, row_top + 8, text_color, COLOR_PANEL, 14, 1)
            else:
                renderer.draw_rect(text_x, row_top + 10, list_w - 16, 10, text_color, filled=False)
