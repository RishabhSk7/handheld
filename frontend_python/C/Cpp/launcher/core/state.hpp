#pragma once

#include <string>

namespace launcher {

class StateManager {
public:
    explicit StateManager(std::string initialState = "splash");

    void transition(const std::string& newState);
    const std::string& getState() const;

private:
    std::string currentState_;
    std::string previousState_;
};

}  // namespace launcher
