import time
import subprocess
from pathlib import Path

import sdl2

from launcher.core.assets import AssetManager
from launcher.core.config import Config
from launcher.core.constants import (
    COLOR_BORDER_GLOW,
    COLOR_DIM,
    COLOR_HIGHLIGHT,
    COLOR_NEON_CYAN,
    COLOR_PANEL,
    COLOR_PANEL_ALT,
    COLOR_TEXT,
    COLOR_TEXT_STRONG,
    UI_BOTTOM_BAR_H,
    UI_MARGIN,
    UI_TOP_BAR_H,
)
from launcher.core.input import InputHandler
from launcher.core.library import RomScanner
from launcher.core.sdl_manager import Renderer, SDL2_Manager
from launcher.core.state import StateManager
from launcher.ui.game_list import GameList
from launcher.ui.main_menu import MainMenu
from launcher.ui.splash import SplashScreen


class App:
    """Main application controller."""

    def __init__(self):
        self.config = Config("config.json")

        self.sdl2_mgr = SDL2_Manager(self.config.window_width, self.config.window_height)
        self.renderer = Renderer(self.sdl2_mgr)

        self.assets = AssetManager(self.sdl2_mgr, self.config.assets_dir)

        rom_scanner = RomScanner(self.config.roms_dir)
        self.game_library = rom_scanner.scan()

        self.main_menu = MainMenu(self.game_library)
        self.game_list = GameList(self.game_library)
        self.splash_screen = SplashScreen(duration_seconds=2.0)

        self.state_manager = StateManager("splash")
        self.input_handler = InputHandler()

        wallpaper_path = Path(__file__).resolve().parent.parent / "resources" / "wallpaper.jpg"
        self.background_texture = sdl2.sdlimage.IMG_LoadTexture(
            self.renderer.renderer.sdlrenderer,
            str(wallpaper_path).encode("utf-8"),
        )
        if not self.background_texture:
            print(f"[WARNING] Failed to load wallpaper: {wallpaper_path}")

        self.running = True

    def handle_input(self):
        """Handle input and convert to state changes."""
        actions = self.input_handler.poll()

        if "quit" in actions:
            self.running = False

        state = self.state_manager.get_state()

        for action in actions:
            if state == "splash":
                pass

            elif state == "main_menu":
                if action == "move_left":
                    self.main_menu.move_left()
                elif action == "move_right":
                    self.main_menu.move_right()
                elif action == "select":
                    system = self.main_menu.get_selected_system()
                    if system:
                        self.game_list.set_system(system)
                        self.state_manager.transition("game_list")
                elif action == "back":
                    self.running = False

            elif state == "game_list":
                if action == "move_up":
                    self.game_list.move_up()
                elif action == "move_down":
                    self.game_list.move_down()
                elif action == "select":
                    rom = self.game_list.get_selected_rom()
                    if rom:
                        self.launch_game(rom)
                elif action == "back":
                    self.state_manager.transition("main_menu")

    def update(self, delta_time: float):
        """Update game state."""
        state = self.state_manager.get_state()

        if state == "splash":
            if self.splash_screen.update():
                self.state_manager.transition("main_menu")

        elif state == "main_menu":
            self.main_menu.update(delta_time)

        elif state == "game_list":
            self.game_list.update(delta_time)

    def _draw_global_overlays(self):
        """Draw synthwave HUD overlays above wallpaper and below screen content."""
        width = self.config.window_width
        height = self.config.window_height

        self.renderer.draw_rect(0, 0, width, UI_TOP_BAR_H, COLOR_PANEL, filled=True)
        self.renderer.draw_rect(0, height - UI_BOTTOM_BAR_H, width, UI_BOTTOM_BAR_H, COLOR_PANEL, filled=True)

        self.renderer.draw_rect(UI_MARGIN, UI_TOP_BAR_H, width - (UI_MARGIN * 2), 1, COLOR_NEON_CYAN, filled=True)
        self.renderer.draw_rect(
            UI_MARGIN,
            height - UI_BOTTOM_BAR_H - 1,
            width - (UI_MARGIN * 2),
            1,
            COLOR_BORDER_GLOW,
            filled=True,
        )

        for y in range(52, height - 36, 12):
            self.renderer.draw_rect(0, y, width, 1, COLOR_PANEL_ALT, filled=True)

    def render(self):
        """Render current screen."""
        self.renderer.clear()
        state = self.state_manager.get_state()

        if self.background_texture:
            self.renderer.draw_texture(
                self.background_texture,
                0,
                0,
                self.config.window_width,
                self.config.window_height,
            )

            if state != "splash":
                self._draw_global_overlays()

        if state == "splash":
            self.splash_screen.render(self.renderer, self.assets)

        elif state == "main_menu":
            self.assets.draw_text_shadowed(
                self.renderer,
                "SYSTEM SELECT",
                14,
                8,
                COLOR_TEXT_STRONG,
                COLOR_PANEL_ALT,
                14,
            )

            self.main_menu.render(self.renderer, self.assets, self.game_library)

            self.assets.draw_text_shadowed(
                self.renderer,
                "LEFT/RIGHT SWITCH  ENTER CONFIRM  ESC EXIT",
                12,
                self.config.window_height - 20,
                COLOR_DIM,
                size=10,
            )

        elif state == "game_list":
            selected_rom = self.game_list.get_selected_rom()
            if selected_rom:
                selected_system = self.game_list.current_system or "unknown"
                selected_title = AssetManager.truncate_text(f"{selected_system}", max_len=28)
                selected_sprite = self.assets.render_text(selected_title.upper(), COLOR_HIGHLIGHT, 14)
                if selected_sprite:
                    centered_x = (self.config.window_width - selected_sprite.size[0]) // 2
                    self.renderer.draw_rect(centered_x - 5, 7, selected_sprite.size[0] + 10, 16, COLOR_PANEL, filled=True)
                    self.renderer.draw_sprite(selected_sprite, centered_x, 10)
                else:
                    self.renderer.draw_rect(95, 10, 130, 12, COLOR_HIGHLIGHT, filled=False)

            self.game_list.render(self.renderer, self.assets)

            self.assets.draw_text_shadowed(
                self.renderer,
                "UP/DOWN MOVE  ENTER LAUNCH  ESC BACK",
                12,
                self.config.window_height - 20,
                COLOR_DIM,
                size=10,
            )

        self.renderer.present()

    def run(self):
        """Main application loop."""
        print("[INFO] Starting handheld launcher...")
        self.splash_screen.start()
        frame_time_ms = 1000 // self.config.target_fps
        last_frame_time = time.time()

        while self.running:
            frame_start = time.time()
            delta_time = frame_start - last_frame_time
            last_frame_time = frame_start

            self.handle_input()
            self.update(delta_time)
            self.render()

            elapsed_ms = (time.time() - frame_start) * 1000
            remaining_ms = frame_time_ms - elapsed_ms
            if remaining_ms > 0:
                sdl2.SDL_Delay(int(remaining_ms))

        self.cleanup()

    def launch_game(self, rom_filename: str):
        """Launch a game with RetroArch."""
        system = self.game_list.current_system
        if not system:
            print("[ERROR] Cannot launch game: no system selected.")
            return

        core_filename = self.config.core_map.get(system)
        if not core_filename:
            print(f"[ERROR] Cannot launch game: no core mapping configured for system '{system}'.")
            return

        core_path = (self.config.cores_dir / system / core_filename).expanduser()
        rom_path = (self.config.roms_dir / system / rom_filename).expanduser()

        if not core_path.exists():
            print(f"[ERROR] Cannot launch game: core file not found: {core_path}")
            return

        if not rom_path.exists():
            print(f"[ERROR] Cannot launch game: ROM file not found: {rom_path}")
            return

        cmd = [self.config.retroarch_bin, "-v", "-L", str(core_path), str(rom_path)]
        print(f"[INFO] Launching: {' '.join(cmd)}")

        try:
            result = subprocess.run(cmd, check=False)
        except FileNotFoundError:
            print(f"[ERROR] Cannot launch game: RetroArch binary not found: {self.config.retroarch_bin}")
            return
        except Exception as exc:
            print(f"[ERROR] Failed to launch RetroArch: {exc}")
            return

        if result.returncode != 0:
            print(f"[ERROR] RetroArch exited with code {result.returncode}.")

    def cleanup(self):
        """Clean up resources."""
        print("[INFO] Cleaning up...")
        if self.background_texture:
            sdl2.SDL_DestroyTexture(self.background_texture)
            self.background_texture = None
        self.assets.cleanup()
        self.sdl2_mgr.cleanup()
        print("[INFO] Goodbye!")
