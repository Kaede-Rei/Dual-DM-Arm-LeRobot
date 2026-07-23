# dm_arm 遥操作（Teleoperation）脚本
#
# 前提：dm_arm 已通过插件机制注册（--robot.type=dm_follower 等可用）
# 使用前确保：
# 1. 已激活包含 LeRobot 和 lerobot_robot_multi_robots 包的 Python 环境
# 2. Leader 臂和 Follower 臂已正确连接并上电
# 3. Leader 臂和 Follower 臂端口已在 config/arm.yaml 中正确配置（可通过参数覆盖）
# 4. 若使用 --display_data，请确保摄像头已连接并在 config/arm.yaml 中正确配置
#
# 使用方法示例：
# python scripts/teleop.py                                    # 无界面纯遥操作（仅关节映射，无摄像头）
# python scripts/teleop.py --display_data                     # 启动 rerun.io GUI + 自动显示摄像头
# python scripts/teleop.py --config config/arm.yaml           # 指定机械臂配置
# python scripts/teleop.py --follower_port /dev/ttyACM1 --leader_port /dev/ttyUSB1 --freq 100
#
# 支持的参数：
# --config <path> 机械臂配置文件（默认 config/arm.yaml）
# --follower_port <port> Follower 臂串口路径（默认读取配置文件）
# --leader_port <port> Leader 臂串口路径（默认读取配置文件）
# --freq <float> 无界面模式下控制循环频率（Hz，默认 200.0）
# --joint_velocity_scaling <float> Follower 关节速度缩放（默认读取配置文件）
# --display_data 若指定，则启动 lerobot-teleoperate GUI 模式（自动启用摄像头显示，不运行脚本主循环）

import argparse
import time
import subprocess
from pathlib import Path
from lerobot_robot_multi_robots.dm_arm import DMFollower, DMLeader
from lerobot_robot_multi_robots.dm_arm import DMFollowerConfig, DMLeaderConfig

DEFAULT_CONFIG_PATH = str(Path(__file__).resolve().parents[1] / "config" / "arm.yaml")


def parse_args():
    ap = argparse.ArgumentParser(description="DK1 teleoperation")
    ap.add_argument("--config", default=DEFAULT_CONFIG_PATH)
    ap.add_argument("--follower_port", default=None)
    ap.add_argument("--leader_port", default=None)
    ap.add_argument("--freq", type=float, default=200.0)
    ap.add_argument(
        "--joint_velocity_scaling",
        type=float,
        default=None,
        help="Follower 关节速度缩放（默认读取配置文件）",
    )
    ap.add_argument(
        "--display_data",
        action="store_true",
        help="若开启，则启动 lerobot-teleoperate GUI 并自动显示摄像头（rerun.io）",
    )
    return ap.parse_args()


def main():
    args = parse_args()

    if args.display_data:
        cmd = [
            "lerobot-teleoperate",
            "--robot.type=dm_follower",
            f"--robot.config_path={args.config}",
            "--teleop.type=dm_leader",
            f"--teleop.config_path={args.config}",
            "--display_data=true",
        ]

        if args.follower_port is not None:
            cmd.append(f"--robot.port={args.follower_port}")
        if args.leader_port is not None:
            cmd.append(f"--teleop.port={args.leader_port}")
        if args.joint_velocity_scaling is not None:
            cmd.append(f"--robot.joint_velocity_scaling={args.joint_velocity_scaling}")

        try:
            subprocess.run(cmd, check=True)
        except KeyboardInterrupt:
            print("\nStopping teleop GUI...")
        return

    # 无界面纯遥操作模式（仅关节动作映射，无摄像头、无 GUI）
    leader = DMLeader(DMLeaderConfig(config_path=args.config, port=args.leader_port))
    leader.connect()
    follower = DMFollower(
        DMFollowerConfig(
            config_path=args.config,
            port=args.follower_port,
            joint_velocity_scaling=args.joint_velocity_scaling,
            disable_torque_on_disconnect=True,
            cameras={},
        )
    )
    follower.connect()

    try:
        period = 1.0 / args.freq
        print("Starting pure teleoperation (no GUI, no cameras)...")
        while True:
            action = leader.get_action()
            follower.send_action(action)
            time.sleep(period)
    except KeyboardInterrupt:
        print("\nStopping pure teleoperation...")
    finally:
        leader.disconnect()
        follower.disconnect()


if __name__ == "__main__":
    main()
