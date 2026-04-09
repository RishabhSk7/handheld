from pathlib import Path
from typing import Dict, List

from .constants import VALID_ROM_EXTENSIONS


class GameLibrary:
    """Represents the game library (systems and ROMs)."""

    def __init__(self, systems: Dict[str, List[str]]):
        self.systems = systems
        self.system_names = sorted(systems.keys())

    def get_systems(self) -> List[str]:
        """Get sorted list of system names."""
        return self.system_names

    def get_roms(self, system: str) -> List[str]:
        """Get ROMs for a specific system."""
        return sorted(self.systems.get(system, []))

    def get_system_display_name(self, system: str) -> str:
        """Convert system folder name to friendly name."""
        names = {
            "nes": "NES",
            "snes": "SNES",
            "gba": "GBA",
            "gb": "Game Boy",
            "gbc": "GBC",
        }
        return names.get(system, system.upper())

    def get_rom_count(self) -> int:
        """Get total ROM count."""
        return sum(len(roms) for roms in self.systems.values())


class RomScanner:
    """Scans ROM directories and builds game library."""

    def __init__(self, roms_dir: Path):
        self.roms_dir = Path(roms_dir)
        self.roms_dir.mkdir(parents=True, exist_ok=True)

    def scan(self) -> GameLibrary:
        """Scan ROM directories and return GameLibrary."""
        systems = {}

        for system_dir in self.roms_dir.iterdir():
            if system_dir.is_dir():
                system_name = system_dir.name
                roms = [
                    f.name
                    for f in system_dir.iterdir()
                    if f.is_file()
                    and not f.name.startswith(".")
                    and f.suffix.lower() in VALID_ROM_EXTENSIONS
                ]
                if roms:
                    systems[system_name] = roms
                    print(f"[INFO] Found system '{system_name}' with {len(roms)} ROM(s)")

        if not systems:
            print("[WARNING] No ROMs found. Library is empty.")

        return GameLibrary(systems)
