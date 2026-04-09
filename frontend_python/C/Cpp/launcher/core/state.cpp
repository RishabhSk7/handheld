#include "launcher/core/state.hpp"

#include <iostream>

namespace launcher {

StateManager::StateManager(std::string initialState)
    : currentState_(std::move(initialState)) {}

void StateManager::transition(const std::string& newState) {
    previousState_ = currentState_;
    currentState_ = newState;
    std::cout << "[STATE] " << previousState_ << " -> " << currentState_ << '\n';
}

const std::string& StateManager::getState() const {
    return currentState_;
}

}  // namespace launcher
