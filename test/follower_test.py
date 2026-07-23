from lerobot_robot_multi_robots.dm_arm import DMFollower
from lerobot_robot_multi_robots.config_dm_arm import DMFollowerConfig
from lerobot_robot_multi_robots.motors.DM_Control_Python.DM_CAN import *

import time
from pathlib import Path

config_path = str(Path(__file__).resolve().parents[1] / "config" / "arm.yaml")
follower_config = DMFollowerConfig(
    config_path=config_path,
    cameras={},
)
follower = DMFollower(follower_config)

follower.connect()

for key, motor in follower.motors.items():
    follower.control.disable(motor)

try:
    while True:
        print(follower.get_observation())
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\nStopping read position...")
    follower.disconnect()
