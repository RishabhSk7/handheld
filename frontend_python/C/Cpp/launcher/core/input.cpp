#include "launcher/core/input.hpp"

#include <SDL2/SDL.h>

namespace launcher {

std::vector<Action> InputHandler::poll() const {
    std::vector<Action> actions;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            actions.push_back(Action::Quit);
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    actions.push_back(Action::MoveLeft);
                    break;
                case SDLK_RIGHT:
                    actions.push_back(Action::MoveRight);
                    break;
                case SDLK_UP:
                    actions.push_back(Action::MoveUp);
                    break;
                case SDLK_DOWN:
                    actions.push_back(Action::MoveDown);
                    break;
                case SDLK_RETURN:
                    actions.push_back(Action::Select);
                    break;
                case SDLK_ESCAPE:
                    actions.push_back(Action::Back);
                    break;
                default:
                    break;
            }
        }
    }

    return actions;
}

}  // namespace launcher
