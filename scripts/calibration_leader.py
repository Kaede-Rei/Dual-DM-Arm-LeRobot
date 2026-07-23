from lerobot_robot_multi_robots.dm_arm import DMLeader
from lerobot_robot_multi_robots.dm_arm import DMLeaderConfig

import argparse
from pathlib import Path

DEFAULT_CONFIG_PATH = str(Path(__file__).resolve().parents[1] / "config" / "arm.yaml")

parser = argparse.ArgumentParser(description="DM Leader calibration")
parser.add_argument("--config", default=DEFAULT_CONFIG_PATH)
parser.add_argument("--leader_port", default=None)
args = parser.parse_args()

leader = DMLeader(DMLeaderConfig(config_path=args.config, port=args.leader_port))
leader.connect()

# 建议先禁用夹爪力矩，避免写寄存器时受力
leader.bus.write("Torque_Enable", "gripper", 0, normalize=False)

# 将所有关节的当前编码值偏移到中点（2048），使当前姿势成为“零”
target_mid = 2048
for motor in leader.bus.motors:
    if motor == "gripper":
        # 夹爪通常不居中到 2048，如需也设零可按需处理
        continue
    pos = leader.bus.read("Present_Position", motor, normalize=False)
    leader.bus.write("Homing_Offset", motor, target_mid - pos, normalize=False)
    print(f"{motor}: Present={pos}, Offset={target_mid - pos}")

# 简单校验，get_action() 包含配置文件中的软件方向与零位偏移
print("Homing_Offset:", leader.bus.sync_read("Homing_Offset", normalize=False))
print("Present_Position:", leader.bus.sync_read("Present_Position", normalize=False))
print("get_action():", leader.get_action())

leader.disconnect()
