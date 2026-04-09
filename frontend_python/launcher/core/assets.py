from pathlib import Path
from typing import Dict, List, Optional, TYPE_CHECKING, Tuple
from ctypes import cast, c_void_p

import sdl2
import sdl2.ext

from .constants import COLOR_TEXT

if TYPE_CHECKING:
    from .sdl_manager import Renderer


class AssetManager:
    """Manages asset loading and caching (images, fonts, text sprites)."""

    def __init__(self, sdl2_manager, assets_dir: Path):
        self.sdl2_mgr = sdl2_manager
        self.assets_dir = assets_dir
        self.renderer = sdl2_manager.get_renderer()
        self.sprite_factory = sdl2_manager.get_sprite_factory()

        self.assets_dir.mkdir(parents=True, exist_ok=True)

        self.font = self._load_font()
        self.text_enabled = self.font is not None
        self.text_cache: Dict[Tuple[str, int], sdl2.ext.Sprite] = {}
        self.image_cache: Dict[str, Optional[sdl2.ext.Sprite]] = {}
        self.logo_sprite = self._load_logo()

        self.rom_placeholder_sprite = self._load_first_image(
            [
                Path("./resources/rom_placeholder.jpg"),
                Path("./resources/rom_placeholder.png"),
                self.assets_dir / "rom_placeholder.jpg",
                self.assets_dir / "rom_placeholder.png",
            ]
        )
        self.game_placeholder_sprite = self._load_first_image(
            [
                Path("./resources/game_placeholder.jpg"),
                Path("./resources/game_placeholder.png"),
                Path("./resources/games_placeholder.jpg"),
                Path("./resources/games_placeholder.png"),
                self.assets_dir / "game_placeholder.jpg",
                self.assets_dir / "game_placeholder.png",
                self.assets_dir / "games_placeholder.jpg",
                self.assets_dir / "games_placeholder.png",
            ]
        )
        self.system_placeholder_sprite = self._load_first_image(
            [
                Path("./resources/system_placeholder.jpg"),
                Path("./resources/system_placeholder.png"),
                self.assets_dir / "system_placeholder.jpg",
                self.assets_dir / "system_placeholder.png",
            ]
        )

    def _load_font(self) -> Optional[sdl2.ext.FontManager]:
        """Load font using FontManager, fallback to default."""
        candidate_paths = [
            self.assets_dir / "font.ttf",
            self.assets_dir / "fonts" / "DejaVuSans.ttf",
            self.assets_dir / "fonts" / "Roboto-Regular.ttf",
            self.assets_dir / "fonts" / "LiberationSans-Regular.ttf",
            Path("./resources/fonts/DejaVuSans.ttf"),
            Path("./resources/fonts/Roboto-Regular.ttf"),
            Path("./resources/fonts/LiberationSans-Regular.ttf"),
            Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
            Path("/usr/share/fonts/dejavu/DejaVuSans.ttf"),
            Path("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"),
            Path("/usr/share/fonts/truetype/freefont/FreeSans.ttf"),
            Path("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"),
        ]

        for font_path in candidate_paths:
            if font_path.exists():
                try:
                    print(f"[INFO] Using font: {font_path}")
                    return sdl2.ext.FontManager(font_path=str(font_path), size=16)
                except Exception as e:
                    print(f"[WARNING] Failed to load font '{font_path}': {e}")

        for root in [Path("/usr/share/fonts"), self.assets_dir, Path("./resources")]:
            if root.exists():
                try:
                    for ttf_file in root.rglob("*.ttf"):
                        try:
                            print(f"[INFO] Using discovered font: {ttf_file}")
                            return sdl2.ext.FontManager(font_path=str(ttf_file), size=16)
                        except Exception:
                            continue
                except Exception:
                    continue

        print("[WARNING] No TTF font found. Text rendering disabled.")
        return None

    def _load_logo(self) -> Optional[sdl2.ext.Sprite]:
        """Load logo image from known locations."""
        possible_paths = [
            Path("./resources/logo.png"),
            Path("../resources/logo.png"),
            Path("../../resources/logo.png"),
            self.assets_dir / "logo.png",
            self.assets_dir / "logo.bmp",
        ]

        for path in possible_paths:
            if path.exists():
                try:
                    sprite = self.sprite_factory.from_image(str(path))
                    print(f"[INFO] Loaded logo from {path}")
                    return sprite
                except Exception as e:
                    print(f"[WARNING] Could not load logo from {path}: {e}")

        print("[WARNING] Logo image not found; splash will be empty.")
        return None

    def _load_image(self, image_path: Path) -> Optional[sdl2.ext.Sprite]:
        """Load and cache any image sprite by absolute path string key."""
        cache_key = str(image_path.resolve())
        if cache_key in self.image_cache:
            return self.image_cache[cache_key]

        if not image_path.exists():
            self.image_cache[cache_key] = None
            return None

        try:
            sprite = self.sprite_factory.from_image(str(image_path))
            self.image_cache[cache_key] = sprite
            return sprite
        except Exception as e:
            print(f"[WARNING] Could not load image {image_path}: {e}")
            self.image_cache[cache_key] = None
            return None

    def _load_first_image(self, candidates: List[Path]) -> Optional[sdl2.ext.Sprite]:
        """Return the first successfully loaded image sprite from candidates."""
        for candidate in candidates:
            sprite = self._load_image(candidate)
            if sprite:
                return sprite
        return None

    def _rom_art_candidates(self, system: str, rom_filename: str, art_type: str) -> List[Path]:
        """Build candidate image paths for ROM art lookup."""
        rom_stem = Path(rom_filename).stem
        rom_full = Path(rom_filename).name
        extensions = (".png", ".jpg", ".jpeg", ".webp", ".bmp")

        candidate_roots = [
            self.assets_dir / art_type / system,
            self.assets_dir / "boxart" / system,
            Path("./resources") / art_type / system,
            Path("./resources") / "boxart" / system,
        ]

        candidates: List[Path] = []
        for root in candidate_roots:
            for ext in extensions:
                candidates.append(root / f"{rom_stem}{ext}")
                candidates.append(root / f"{rom_full}{ext}")

        return candidates

    def _system_icon_candidates(self, system: str) -> List[Path]:
        """Build candidate image paths for system icon lookup."""
        system_names = [system, system.lower(), system.upper()]
        extensions = (".png", ".jpg", ".jpeg", ".webp", ".bmp")

        candidate_roots = [
            self.assets_dir / "systems",
            self.assets_dir / "icons" / "systems",
            self.assets_dir / "icons",
            Path("./resources") / "systems",
            Path("./resources") / "icons" / "systems",
            Path("./resources") / "icons",
        ]

        candidates: List[Path] = []
        for root in candidate_roots:
            for system_name in system_names:
                for ext in extensions:
                    candidates.append(root / f"{system_name}{ext}")

        return candidates

    def get_system_icon_sprite(self, system: Optional[str]) -> Optional[sdl2.ext.Sprite]:
        """Get icon sprite for a system with placeholder fallback."""
        if system:
            sprite = self._load_first_image(self._system_icon_candidates(system))
            if sprite:
                return sprite
        return self.system_placeholder_sprite

    def get_rom_icon_sprite(self, system: Optional[str], rom_filename: Optional[str]) -> Optional[sdl2.ext.Sprite]:
        """Get small icon sprite for a ROM with placeholder fallback."""
        if system and rom_filename:
            sprite = self._load_first_image(self._rom_art_candidates(system, rom_filename, "icons"))
            if sprite:
                return sprite
        return self.rom_placeholder_sprite

    def get_rom_preview_sprite(self, system: Optional[str], rom_filename: Optional[str]) -> Optional[sdl2.ext.Sprite]:
        """Get right-panel preview sprite for a ROM with placeholder fallback."""
        if system and rom_filename:
            sprite = self._load_first_image(self._rom_art_candidates(system, rom_filename, "boxart"))
            if sprite:
                return sprite
        return self.game_placeholder_sprite

    @staticmethod
    def truncate_text(text: str, max_len: int = 20) -> str:
        """Truncate text to fit screen width."""
        return text[:max_len] + "..." if len(text) > max_len else text

    def render_text(self, text: str, color: sdl2.SDL_Color = COLOR_TEXT, size: int = 16) -> Optional[sdl2.ext.Sprite]:
        """Render text to a sprite and cache it."""
        cache_key = (text, size, color.r, color.g, color.b, color.a)

        if cache_key in self.text_cache:
            return self.text_cache[cache_key]

        if not self.font:
            return None

        try:
            text_sprite = self.sprite_factory.from_text(
                text,
                fontmanager=self.font,
                size=size,
                color=color,
            )
            self.text_cache[cache_key] = text_sprite
            return text_sprite
        except Exception as e:
            print(f"[ERROR] Failed to render text '{text}': {e}")
            return None

    def draw_text_or_placeholder(
        self,
        renderer: "Renderer",
        text: str,
        x: int,
        y: int,
        color: sdl2.SDL_Color = COLOR_TEXT,
        size: int = 16,
        placeholder_w: int = 90,
        placeholder_h: int = 12,
    ):
        """Draw text when available, otherwise draw a placeholder bar."""
        sprite = self.render_text(text, color, size)
        if sprite:
            renderer.draw_sprite(sprite, x, y)
            return

        renderer.draw_rect(x, y, placeholder_w, placeholder_h, color, filled=False)

    def draw_text_shadowed(
        self,
        renderer: "Renderer",
        text: str,
        x: int,
        y: int,
        fg_color: sdl2.SDL_Color = COLOR_TEXT,
        shadow_color: Optional[sdl2.SDL_Color] = None,
        size: int = 16,
        offset: int = 1,
    ):
        """Draw text with a simple shadow pass for readability."""
        if shadow_color is None:
            shadow_color = sdl2.SDL_Color(12, 8, 28, 255)

        shadow_sprite = self.render_text(text, shadow_color, size)
        if shadow_sprite:
            renderer.draw_sprite(shadow_sprite, x + offset, y + offset)

        self.draw_text_or_placeholder(
            renderer,
            text,
            x,
            y,
            fg_color,
            size,
            placeholder_w=max(48, len(text) * 6),
            placeholder_h=max(10, size // 2),
        )

    def cleanup(self):
        """Clean up cached assets."""
        destroyed_textures = set()

        all_sprites = list(self.text_cache.values()) + [
            self.logo_sprite,
            self.rom_placeholder_sprite,
            self.game_placeholder_sprite,
            self.system_placeholder_sprite,
        ] + [sprite for sprite in self.image_cache.values() if sprite]

        for sprite in all_sprites:
            if not sprite:
                continue

            texture = getattr(sprite, "texture", None)
            texture_addr = cast(texture, c_void_p).value if texture else None
            if texture_addr and texture_addr not in destroyed_textures:
                sdl2.SDL_DestroyTexture(texture)
                destroyed_textures.add(texture_addr)

        self.logo_sprite = None
        self.rom_placeholder_sprite = None
        self.game_placeholder_sprite = None
        self.system_placeholder_sprite = None

        self.text_cache.clear()
        self.image_cache.clear()
