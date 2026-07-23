from dataclasses import dataclass

from lerobot.robots import RobotConfig
from lerobot.teleoperators import TeleoperatorConfig
from lerobot.cameras import CameraConfig

from .config_loader import load_camera_configs, load_config_section


@RobotConfig.register_subclass("dm_follower")
@dataclass
class DMFollowerConfig(RobotConfig):
    config_path: str | None = None
    port: str | None = None
    disable_torque_on_disconnect: bool | None = None
    joint_velocity_scaling: float | None = None
    max_gripper_torque: float | None = None
    cameras: dict[str, CameraConfig] | None = None

    def __post_init__(self):
        config = load_config_section(self.config_path, "robot")

        if self.port is None:
            self.port = config.get("port")
        if self.disable_torque_on_disconnect is None:
            self.disable_torque_on_disconnect = config.get("disable_torque_on_disconnect", True)
        if self.joint_velocity_scaling is None:
            self.joint_velocity_scaling = config.get("joint_velocity_scaling", 0.2)
        if self.max_gripper_torque is None:
            self.max_gripper_torque = config.get("max_gripper_torque", 1.0)
        if self.cameras is None:
            self.cameras = load_camera_configs(config.get("cameras", {}))

        if not self.port:
            raise ValueError("Follower port must be configured")
        if not 0.0 < self.joint_velocity_scaling <= 1.0:
            raise ValueError("joint_velocity_scaling must be in (0, 1]")
        if self.max_gripper_torque <= 0.0:
            raise ValueError("max_gripper_torque must be greater than 0")

        super().__post_init__()


@TeleoperatorConfig.register_subclass("dm_leader")
@dataclass
class DMLeaderConfig(TeleoperatorConfig):
    config_path: str | None = None
    port: str | None = None
    gripper_open_pos: int | None = None
    gripper_closed_pos: int | None = None
    direction: list[int] | None = None
    offset: list[float] | None = None

    def __post_init__(self):
        config = load_config_section(self.config_path, "teleop")

        if self.port is None:
            self.port = config.get("port")
        if self.gripper_open_pos is None:
            self.gripper_open_pos = config.get("gripper_open_pos", 2280)
        if self.gripper_closed_pos is None:
            self.gripper_closed_pos = config.get("gripper_closed_pos", 1670)
        if self.direction is None:
            self.direction = config.get("direction", [1, 1, 1, 1, 1, 1])
        if self.offset is None:
            self.offset = config.get("offset", [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])

        if not self.port:
            raise ValueError("Leader port must be configured")
        if len(self.direction) != 6:
            raise ValueError("direction must contain 6 values")
        if any(value not in (-1, 1) for value in self.direction):
            raise ValueError("direction values must be 1 or -1")
        if len(self.offset) != 6:
            raise ValueError("offset must contain 6 values")
        if self.gripper_open_pos == self.gripper_closed_pos:
            raise ValueError("gripper_open_pos and gripper_closed_pos must be different")

        self.direction = [int(value) for value in self.direction]
        self.offset = [float(value) for value in self.offset]
