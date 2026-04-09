from typing import List

import sdl2
import sdl2.ext


class InputHandler:
    """Handles keyboard input and maps to actions."""

    def __init__(self):
        self.actions = []

    def poll(self) -> List[str]:
        """
        Poll SDL2 events and return list of actions.
        Returns: ["move_left", "move_right", "move_up", "move_down", "select", "back", "quit"]
        """
        self.actions = []

        for event in sdl2.ext.get_events():
            if event.type == sdl2.SDL_QUIT:
                self.actions.append("quit")
            elif event.type == sdl2.SDL_KEYDOWN:
                key = event.key.keysym.sym
                if key == sdl2.SDLK_LEFT:
                    self.actions.append("move_left")
                elif key == sdl2.SDLK_RIGHT:
                    self.actions.append("move_right")
                elif key == sdl2.SDLK_UP:
                    self.actions.append("move_up")
                elif key == sdl2.SDLK_DOWN:
                    self.actions.append("move_down")
                elif key == sdl2.SDLK_RETURN:
                    self.actions.append("select")
                elif key == sdl2.SDLK_ESCAPE:
                    self.actions.append("back")

        return self.actions
