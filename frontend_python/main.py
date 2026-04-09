#!/usr/bin/env python3
"""Entry point for the handheld launcher."""

import sys
import traceback

from launcher.app import App


def main():
    """Run the launcher application."""
    try:
        app = App()
        app.run()
    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user")
    except Exception as e:
        print(f"[ERROR] Fatal error: {e}")
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
