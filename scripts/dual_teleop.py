# dual_dm_arm 遥操作（Teleoperation）脚本
#
# 前提：dual_dm_arm 已通过插件机制注册（--robot.type=dual_dm_follower 等可用）
# 使用前确保：
# 1. 已激活包含 LeRobot 和 lerobot_robot_multi_robots 包的 Python 环境
# 2. Leader 左臂和右臂、Follower 左臂和右臂已正确连接并上电
# 3. 左右机械臂端口已在 config/dual_arm.yaml 中正确配置（可通过参数覆盖）
# 4. 若使用 --display_data，请确保摄像头已连接并在 config/dual_arm.yaml 中正确配置
#
# 使用方法示例：
# python scripts/dual_teleop.py                                  # 无界面纯遥操作（仅关节映射，无摄像头）
# python scripts/dual_teleop.py --display_data                   # 启动 rerun.io GUI + 自动显示摄像头
# python scripts/dual_teleop.py --config config/dual_arm.yaml    # 指定双臂配置
# python scripts/dual_teleop.py --follower_left_port /dev/ttyACM2 --follower_right_port /dev/ttyACM3 \
#     --leader_left_port /dev/ttyUSB2 --leader_right_port /dev/ttyUSB3 --freq 100
#
# 支持的参数：
# --config <path> 双臂配置文件（默认 config/dual_arm.yaml）
# --follower_left_port <port> Follower 左臂串口路径（默认读取配置文件）
# --follower_right_port <port> Follower 右臂串口路径（默认读取配置文件）
# --leader_left_port <port> Leader 左臂串口路径（默认读取配置文件）
# --leader_right_port <port> Leader 右臂串口路径（默认读取配置文件）
# --freq <float> 无界面模式下控制循环频率（Hz，默认 200.0）
# --joint_velocity_scaling <float> Follower 关节速度缩放（默认读取配置文件）
# --display_data 若指定，则启动 lerobot-teleoperate GUI 模式（自动启用摄像头显示，不运行脚本主循环）

import argparse
import time
import subprocess
from pathlib import Path
from lerobot_robot_multi_robots.dual_dm_arm import DualDMFollower, DualDMLeader
from lerobot_robot_multi_robots.dual_dm_arm import (
    DualDMFollowerConfig,
    DualDMLeaderConfig,
)

DEFAULT_CONFIG_PATH = str(Path(__file__).resolve().parents[1] / "config" / "dual_arm.yaml")


def parse_args():
    ap = argparse.ArgumentParser(description="Dual DK1 teleoperation")
    ap.add_argument("--config", default=DEFAULT_CONFIG_PATH)
    ap.add_argument("--follower_left_port", default=None)
    ap.add_argument("--follower_right_port", default=None)
    ap.add_argument("--leader_left_port", default=None)
    ap.add_argument("--leader_right_port", default=None)
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
            "--robot.type=dual_dm_follower",
            f"--robot.config_path={args.config}",
            "--teleop.type=dual_dm_leader",
            f"--teleop.config_path={args.config}",
            "--display_data=true",
        ]

        if args.follower_left_port is not None:
            cmd.append(f"--robot.left_port={args.follower_left_port}")
        if args.follower_right_port is not None:
            cmd.append(f"--robot.right_port={args.follower_right_port}")
        if args.leader_left_port is not None:
            cmd.append(f"--teleop.left_port={args.leader_left_port}")
        if args.leader_right_port is not None:
            cmd.append(f"--teleop.right_port={args.leader_right_port}")
        if args.joint_velocity_scaling is not None:
            cmd.append(f"--robot.joint_velocity_scaling={args.joint_velocity_scaling}")

        try:
            subprocess.run(cmd, check=True)
        except KeyboardInterrupt:
            print("\nStopping teleop GUI...")
        return

    # 无界面纯遥操作模式（仅关节动作映射，无摄像头、无 GUI）
    leader = DualDMLeader(
        DualDMLeaderConfig(
            config_path=args.config,
            left_port=args.leader_left_port,
            right_port=args.leader_right_port,
        )
    )
    leader.connect()
    follower = DualDMFollower(
        DualDMFollowerConfig(
            config_path=args.config,
            left_port=args.follower_left_port,
            right_port=args.follower_right_port,
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
