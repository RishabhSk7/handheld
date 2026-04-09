class StateManager:
    """Manages state transitions in the application."""

    def __init__(self, initial_state: str = "splash"):
        self.current_state = initial_state
        self.previous_state = None

    def transition(self, new_state: str):
        """Transition to a new state."""
        self.previous_state = self.current_state
        self.current_state = new_state
        print(f"[STATE] {self.previous_state} -> {self.current_state}")

    def get_state(self) -> str:
        """Get current state."""
        return self.current_state
