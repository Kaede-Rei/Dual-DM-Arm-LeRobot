from dataclasses import dataclass

from lerobot.robots import RobotConfig
from lerobot.teleoperators import TeleoperatorConfig
from lerobot.cameras import CameraConfig

from .config_loader import load_camera_configs, load_config_section


@RobotConfig.register_subclass("dual_dm_follower")
@dataclass
class DualDMFollowerConfig(RobotConfig):
    config_path: str | None = None
    left_port: str | None = None
    right_port: str | None = None
    disable_torque_on_disconnect: bool | None = None
    joint_velocity_scaling: float | None = None
    max_gripper_torque: float | None = None
    cameras: dict[str, CameraConfig] | None = None

    def __post_init__(self):
        config = load_config_section(self.config_path, "robot")

        if self.left_port is None:
            self.left_port = config.get("left_port")
        if self.right_port is None:
            self.right_port = config.get("right_port")
        if self.disable_torque_on_disconnect is None:
            self.disable_torque_on_disconnect = config.get("disable_torque_on_disconnect", True)
        if self.joint_velocity_scaling is None:
            self.joint_velocity_scaling = config.get("joint_velocity_scaling", 0.2)
        if self.max_gripper_torque is None:
            self.max_gripper_torque = config.get("max_gripper_torque", 1.0)
        if self.cameras is None:
            self.cameras = load_camera_configs(config.get("cameras", {}))

        if not self.left_port:
            raise ValueError("Left follower port must be configured")
        if not self.right_port:
            raise ValueError("Right follower port must be configured")
        if not 0.0 < self.joint_velocity_scaling <= 1.0:
            raise ValueError("joint_velocity_scaling must be in (0, 1]")
        if self.max_gripper_torque <= 0.0:
            raise ValueError("max_gripper_torque must be greater than 0")

        super().__post_init__()


@TeleoperatorConfig.register_subclass("dual_dm_leader")
@dataclass
class DualDMLeaderConfig(TeleoperatorConfig):
    config_path: str | None = None
    left_port: str | None = None
    right_port: str | None = None
    gripper_open_pos: int | None = None
    gripper_closed_pos: int | None = None
    left_direction: list[int] | None = None
    left_offset: list[float] | None = None
    right_direction: list[int] | None = None
    right_offset: list[float] | None = None

    def __post_init__(self):
        config = load_config_section(self.config_path, "teleop")

        if self.left_port is None:
            self.left_port = config.get("left_port")
        if self.right_port is None:
            self.right_port = config.get("right_port")
        if self.gripper_open_pos is None:
            self.gripper_open_pos = config.get("gripper_open_pos", 2280)
        if self.gripper_closed_pos is None:
            self.gripper_closed_pos = config.get("gripper_closed_pos", 1670)
        if self.left_direction is None:
            self.left_direction = config.get("left_direction", [1, 1, 1, 1, 1, 1])
        if self.left_offset is None:
            self.left_offset = config.get("left_offset", [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
        if self.right_direction is None:
            self.right_direction = config.get("right_direction", [1, 1, 1, 1, 1, 1])
        if self.right_offset is None:
            self.right_offset = config.get("right_offset", [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])

        if not self.left_port:
            raise ValueError("Left leader port must be configured")
        if not self.right_port:
            raise ValueError("Right leader port must be configured")
        if len(self.left_direction) != 6 or len(self.right_direction) != 6:
            raise ValueError("Leader direction must contain 6 values")
        if any(value not in (-1, 1) for value in self.left_direction):
            raise ValueError("left_direction values must be 1 or -1")
        if any(value not in (-1, 1) for value in self.right_direction):
            raise ValueError("right_direction values must be 1 or -1")
        if len(self.left_offset) != 6 or len(self.right_offset) != 6:
            raise ValueError("Leader offset must contain 6 values")
        if self.gripper_open_pos == self.gripper_closed_pos:
            raise ValueError("gripper_open_pos and gripper_closed_pos must be different")

        self.left_direction = [int(value) for value in self.left_direction]
        self.left_offset = [float(value) for value in self.left_offset]
        self.right_direction = [int(value) for value in self.right_direction]
        self.right_offset = [float(value) for value in self.right_offset]
