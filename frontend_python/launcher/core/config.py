import json
import sys
from pathlib import Path
from typing import Dict


class Config:
    """Load and cache configuration from config.json."""

    def __init__(self, config_path: str = "config.json"):
        self.config_path = config_path
        self.data = self._load_config()

    def _load_config(self) -> Dict:
        """Load configuration from JSON file."""
        try:
            with open(self.config_path, "r") as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"[ERROR] Config file not found: {self.config_path}")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"[ERROR] Invalid JSON in config: {e}")
            sys.exit(1)

    def get(self, key: str, default=None):
        """Get configuration value by key."""
        return self.data.get(key, default)

    @property
    def roms_dir(self) -> Path:
        return Path(self.get("roms_dir", "./roms")).expanduser()

    @property
    def cores_dir(self) -> Path:
        return Path(self.get("cores_dir", "./cores")).expanduser()

    @property
    def assets_dir(self) -> Path:
        return Path(self.get("assets_dir", "./assets")).expanduser()

    @property
    def retroarch_bin(self) -> str:
        return str(self.get("retroarch_bin", "retroarch"))

    @property
    def core_map(self) -> Dict[str, str]:
        mapping = self.get("core_map", {})
        if isinstance(mapping, dict):
            return {str(system): str(core) for system, core in mapping.items()}
        return {}

    @property
    def window_width(self) -> int:
        return self.get("window_width", 320)

    @property
    def window_height(self) -> int:
        return self.get("window_height", 240)

    @property
    def target_fps(self) -> int:
        return self.get("target_fps", 60)
