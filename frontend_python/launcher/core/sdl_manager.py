import sdl2
import sdl2.ext
from ctypes import byref, c_int

from .constants import COLOR_BG, SCREEN_HEIGHT, SCREEN_WIDTH


class SDL2_Manager:
    """Manages SDL2 initialization, window, and renderer."""

    def __init__(self, width: int = SCREEN_WIDTH, height: int = SCREEN_HEIGHT):
        self.width = width
        self.height = height
        self.window = None
        self.renderer = None
        self.sprite_factory = None
        self._init_sdl2()

    def _init_sdl2(self):
        """Initialize SDL2 video subsystem and create window + renderer."""
        if sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO) < 0:
            raise RuntimeError(f"Failed to initialize SDL2: {sdl2.SDL_GetError()}")

        self.window = sdl2.ext.Window(
            "Handheld Launcher",
            size=(self.width, self.height),
            flags=sdl2.SDL_WINDOW_FULLSCREEN,
        )
        self.window.show()

        renderer_errors = []
        self.renderer = None

        for renderer_flags in (
            sdl2.SDL_RENDERER_ACCELERATED,
            0,
            sdl2.SDL_RENDERER_SOFTWARE,
        ):
            try:
                self.renderer = sdl2.ext.Renderer(self.window, flags=renderer_flags)
                break
            except Exception as e:
                renderer_errors.append(str(e))

        if self.renderer is None:
            raise RuntimeError(
                "Failed to create SDL renderer. Tried accelerated/default/software. "
                + " | ".join(renderer_errors)
            )

        self.sprite_factory = sdl2.ext.SpriteFactory(sdl2.ext.TEXTURE, renderer=self.renderer)
        self.sprite_renderer = sdl2.ext.TextureSpriteRenderSystem(self.renderer)

    def get_renderer(self):
        """Return the SDL renderer."""
        return self.renderer

    def get_sprite_factory(self):
        """Return the sprite factory."""
        return self.sprite_factory

    def get_window(self):
        """Return the SDL window."""
        return self.window

    def get_sprite_renderer(self):
        """Return the sprite render system."""
        return self.sprite_renderer

    def cleanup(self):
        """Clean up SDL2 resources."""
        if self.window:
            self.window.close()
        sdl2.SDL_Quit()


class Renderer:
    """Helper for rendering operations."""

    def __init__(self, sdl2_manager: SDL2_Manager):
        self.sdl2_mgr = sdl2_manager
        self.renderer = sdl2_manager.get_renderer()
        self.sprite_renderer = sdl2_manager.get_sprite_renderer()

    def clear(self):
        """Clear screen with background color."""
        self.renderer.clear(COLOR_BG)

    def present(self):
        """Present the rendered frame."""
        self.renderer.present()

    def draw_rect(self, x: int, y: int, width: int, height: int, color: sdl2.SDL_Color, filled: bool = True):
        """Draw a rectangle."""
        sdl2.SDL_SetRenderDrawColor(
            self.renderer.sdlrenderer, color.r, color.g, color.b, color.a
        )
        rect = sdl2.SDL_Rect(x, y, width, height)
        if filled:
            sdl2.SDL_RenderFillRect(self.renderer.sdlrenderer, rect)
        else:
            sdl2.SDL_RenderDrawRect(self.renderer.sdlrenderer, rect)

    def draw_sprite(self, sprite, x: int, y: int):
        """Draw a sprite at position."""
        dst_rect = sdl2.SDL_Rect(x, y, sprite.size[0], sprite.size[1])
        sdl2.SDL_RenderCopy(
            self.renderer.sdlrenderer,
            sprite.texture,
            None,
            dst_rect,
        )

    def draw_texture(
        self,
        texture,
        x: int,
        y: int,
        width: int,
        height: int,
        crop_to_square: bool = False,
    ):
        """Draw a texture scaled to destination rectangle, optionally center-cropping to square."""
        dst_rect = sdl2.SDL_Rect(x, y, width, height)

        src_rect = None
        if crop_to_square:
            tex_w = c_int()
            tex_h = c_int()
            if sdl2.SDL_QueryTexture(texture, None, None, byref(tex_w), byref(tex_h)) == 0:
                crop_side = min(tex_w.value, tex_h.value)
                src_x = (tex_w.value - crop_side) // 2
                src_y = (tex_h.value - crop_side) // 2
                src_rect = sdl2.SDL_Rect(src_x, src_y, crop_side, crop_side)

        sdl2.SDL_RenderCopy(
            self.renderer.sdlrenderer,
            texture,
            src_rect,
            dst_rect,
        )
