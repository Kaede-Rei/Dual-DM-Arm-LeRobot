from pathlib import Path
from typing import Any

import yaml

from lerobot.cameras import CameraConfig
from lerobot.cameras.opencv import OpenCVCameraConfig


def load_config(config_path: str | None) -> dict[str, Any]:
    if config_path is None:
        return {}

    path = Path(config_path).expanduser()
    if not path.is_file():
        raise FileNotFoundError(f"Config file does not exist: {path}")

    with path.open("r", encoding="utf-8") as f:
        config = yaml.safe_load(f)

    if config is None:
        return {}
    if not isinstance(config, dict):
        raise ValueError(f"Config root must be a mapping: {path}")

    return config


def load_config_section(config_path: str | None, section: str) -> dict[str, Any]:
    config = load_config(config_path)
    section_config = config.get(section, {})

    if not isinstance(section_config, dict):
        raise ValueError(f"Config section '{section}' must be a mapping")

    return section_config


def load_camera_configs(config: dict[str, Any]) -> dict[str, CameraConfig]:
    cameras = {}

    for name, camera_config in config.items():
        if isinstance(camera_config, CameraConfig):
            cameras[name] = camera_config
            continue
        if not isinstance(camera_config, dict):
            raise ValueError(f"Camera config '{name}' must be a mapping")

        camera_config = camera_config.copy()
        camera_type = camera_config.pop("type", "opencv")
        if camera_type != "opencv":
            raise ValueError(f"Unsupported camera type '{camera_type}' for '{name}'")

        cameras[name] = OpenCVCameraConfig(**camera_config)

    return cameras
