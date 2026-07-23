from lerobot_robot_multi_robots.dm_arm import DMLeader
from lerobot_robot_multi_robots.config_dm_arm import DMLeaderConfig
import time
from pathlib import Path

config_path = str(Path(__file__).resolve().parents[1] / "config" / "arm.yaml")
leader_config = DMLeaderConfig(config_path=config_path)

leader = DMLeader(leader_config)
leader.connect()


while True:
    action = leader.get_action()
    print(action)
    time.sleep(0.02)
