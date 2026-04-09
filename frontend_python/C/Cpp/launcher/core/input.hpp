#pragma once

#include <vector>

namespace launcher {

enum class Action {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Select,
    Back,
    Quit,
};

class InputHandler {
public:
    std::vector<Action> poll() const;
};

}  // namespace launcher
