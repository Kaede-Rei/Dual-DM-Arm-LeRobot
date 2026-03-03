<div align="center">

# DM-Arm-LeRobot: 双机械臂模仿学习系统

一个基于 [LeRobot](https://github.com/huggingface/lerobot) 的完整双机械臂机器人框架，集成了从遥操作数据采集、数据处理、模型训练到策略推理的全流程

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Ubuntu 20.04/22.04](https://img.shields.io/badge/OS-Ubuntu-orange.svg)](https://ubuntu.com/)
[![Framework: LeRobot](https://img.shields.io/badge/Framework-LeRobot-E7352C.svg)](https://github.com/huggingface/lerobot)
[![Hardware: DM Motor](https://img.shields.io/badge/Hardware-DM--Motor-1F2D3A.svg)](http://www.damiaotech.com/)

</div>

## 🤖 系统架构

### 硬件配置

<img src="./README.assets/dual-arm.jpg" width="60%" alt="dual-arm">

| 部分 | 组件 | 功能 |
|------|------|------|
| **主机** | Ubuntu20.04/22.04 | 主控 |
| **主臂** | Dynamixel 电机 × 6 | 示教手臂（教学型） |
| **从臂** | 达妙（DM）电机 × 6 | 执行臂（学习控制） |
| **夹爪** | DM 系列制动马达 | 抓取控制 |
| **感知** | USB 摄像头 × 2/3 | 全局视角 + 末端视角 |

### 软件栈

- **学习框架**: [LeRobot](https://github.com/huggingface/lerobot)
- **训练策略**: 
  - **ACT** (Action Chunking Transformers): 适合需要长序列规划的复杂任务
  - **SmolVLA** (Small Vision Language Action): 支持自然语言任务描述
- **硬件接口**: 
  - Dynamixel: DynamixelMotorsBus (USB/RS485)
  - 达妙 DM: CAN-bus 接口
- **数据集**: Hugging Face Hub

## 📋 快速开始

### 1. 环境准备

#### 系统依赖
```bash
# Ubuntu 20.04+ 系统
sudo apt install v4l-utils                      # 摄像头工具
sudo usermod -aG dialout $USER                  # 串口权限
sudo reboot                                     # 重启使权限生效
```

#### Python 环境
```bash
# 激活虚拟环境（以移动硬盘为例）
. /media/$USER/AgroTech/home/activate.sh lerobot

# 或手动安装依赖
pip install -e .
```

>   *使用移动硬盘时，将 activate.sh + activate-new-device.sh 放入 home/ 目录，并在 home/ 目录安装*
>   *micromamba 以使用虚拟环境以及在 home/ 目录下存储缓存*

### 2. 硬件连接

#### 电源连接
- **从臂 (DM)**: 24V 小电源 → 转接板 → 底座
- **主臂 (Dynamixel)**: 开关电源 5V 输出

#### 通信连接
- **从臂**: USB-to-CAN 模块 → `/dev/ttyACM*` (默认)
- **主臂**: Type-C 接口 → `/dev/ttyUSB*` (默认)
- **摄像头**: USB 直连 → `/dev/video*`

#### 端口配置
```bash
# 查看设备端口
ls -l /dev/ttyACM*  		# 从臂
ls -l /dev/ttyUSB*  		# 主臂

# 创建固定端口符号链接（可选）
sudo ./bash/usb-port-create.sh

# 验证摄像头
python scripts/get_uvc_cam_idx.py
```

### 3. 基础测试

```bash
# 测试从臂连接
python scripts/follower_test.py

# 测试主臂连接
python scripts/leader_test.py

# 单臂遥操作
python scripts/teleop.py

# 双臂遥操作
python scripts/dual_teleop.py
```

## 📊 完整工作流程

### 阶段 1: 数据采集

#### 配置参数
编辑 `bash/record-dm.sh`:
```bash
FOLLOWER_PORT="/dev/ttyACM0"            # 从臂串口
LEADER_PORT="/dev/ttyUSB0"              # 主臂串口
CAMERAS_CONFIG='{ ... }'                # 摄像头配置
NUM_EPISODES=50                         # 采集轮数
EPISODE_TIME_S=120                      # 单个轮次时长（秒）
TASK_DESCRIPTION="..."                  # 任务描述
```

#### 启动数据采集
```bash
chmod +x bash/record-dm.sh
./bash/record-dm.sh --repo_id $USER/dm_dataset

# 双臂采集
./bash/dual-record-dm.sh --repo_id $USER/dm_dual_dataset
```

#### 采集操作
- **开始**: 听到 `"Record episode X "` 时开始动作
- **重置**: 听到 `"Reset the environment"` 时恢复场景
- **导航键**:
  - `→` : 提前结束当前 episode
  - `←` : 重新录制当前 episode
  - `ESC` : 终止采集

#### 采集建议
- 至少采集 **50 个 episode** (基础: 5 个场景 × 10 例)
- 推荐配置: 1 个全局视角相机 + 1/2 个末端视角相机
- 操作流畅无突变，仅通过摄像头完成任务

### 阶段 2: 数据处理

```bash
# 1. 将数据复制到本地
cp -r ~/.cache/huggingface/lerobot/<repo_id>/data/chunk-*/file-*.parquet ./dataset/data/

# 2. 数据平滑 + 线性插值
python scripts/process_data.py

# 3. 验证波形（查看平滑效果）
python scripts/process_meta.py --data_dir dataset/data/
```

### 阶段 3: 模型训练

#### ACT 策略训练
```bash
./bash/train-dm.sh \
    --policy_repo_id $USER/act_dm_model \
    --dataset_repo_id $USER/dm_dataset \
```

#### SmolVLA 策略训练
```bash
./bash/train-dm.sh \
    --policy_repo_id $USER/smolvla_dm_model \
    --dataset_repo_id $USER/dm_dataset \
```

#### 训练参数指南

| 参数 | 数据量 | 推荐值 | 说明 |
|------|--------|-----------|------|
| `--batch_size` | 通用 | 2-16 | 显存不足时降低；过小易欠拟合 |
| `--steps` | < 50 episodes | 20k-30k | 数据量少时避免过拟合 |
| | 50-200 episodes | 30k-50k | 中等数据量 |
| | > 200 episodes | 50k+ | 充足数据量 |
| `--n_action_steps` | 通用 | 50 | 预测序列长度（数据块大小） |

#### 监控训练
```bash
# 查看日志
tail -f ./log/train_<model_name>.log

# 使用 Weights & Biases（可选）
./bash/train-dm.sh ... --wandb.enable=true
```

### 阶段 4: 模型评估

```bash
# 单臂 ACT 评估
./bash/eval-dm.sh --policy_repo_id $USER/act_dm_model

# 双臂 SmolVLA 评估
./bash/eval-dm.sh --policy_repo_id $USER/smolvla_dm_model
```

#### 评估参数

| 参数 | 说明 |
|------|------|
| `--joint_velocity_scaling` | 关节速度缩放 (0.1 = 10% 速度, 默认 0.1) |
| `--n_action_steps` | 执行动作步数 (1 启用平滑集成) |
| `--temporal_ensemble_coeff` | 低通滤波系数 (越小越平滑) |

### 阶段 5: 策略推理

#### ACT 推理
```bash
python scripts/infer_dm.py
```

#### SmolVLA 推理

```bash
python scripts/infer_dm.py --model_type smolvla
```

#### 双臂推理
```bash
python scripts/infer_dm.py --dual_arm
```

## 🛠️ 工具脚本快速参考

### 实时控制脚本

| 脚本 | 功能 | 命令 |
|------|------|------|
| `teleop.py` | 单臂主从遥操作 | `python scripts/teleop.py` |
| `dual_teleop.py` | 双臂主从遥操作 | `python scripts/dual_teleop.py` |
| `follower_test.py` | 从臂连接测试 | `python scripts/follower_test.py` |
| `leader_test.py` | 主臂连接测试 | `python scripts/leader_test.py` |
| `reset.py` | 机械臂复位 | `python scripts/reset.py` |

### 数据处理脚本

| 脚本 | 功能 | 命令 |
|------|------|------|
| `get_uvc_cam_idx.py` | 扫描摄像头信息 | `python scripts/get_uvc_cam_idx.py` |
| `process_data.py` | 数据平滑+插值 | `python scripts/process_data.py --input_parquet <file>` |
| `process_meta.py` | 更新数据集统计 | `python scripts/process_meta.py --data_dir <dir>` |

### 批量采集脚本

| 脚本 | 功能 | 命令 |
|------|------|------|
| `bash/record-dm.sh` | 单臂数据采集 | `./bash/record-dm.sh --repo_id <id>` |
| `bash/dual-record-dm.sh` | 双臂数据采集 | `./bash/dual-record-dm.sh --repo_id <id>` |
| `bash/train-dm.sh` | 模型训练 | `./bash/train-dm.sh --policy_repo_id <id> --dataset_repo_id <id>` |
| `bash/eval-dm.sh` | 模型评估 | `./bash/eval-dm.sh --policy_repo_id <id>` |

## 📁 项目结构

```
dm-arm/
├── bash/                           # 批处理脚本
│   ├── record-dm.sh                # 数据采集
│   ├── dual-record-dm.sh           # 双臂采集
│   ├── train-dm.sh                 # 模型训练
│   ├── eval-dm.sh                  # 模型评估
│   └── usb-port-create.sh          # 端口配置
├── lerobot_robot_multi_robots/     # 机器人驱动包
│   ├── dm_arm.py                   # 单臂驱动 (DMFollower, DMLeader)
│   ├── dual_dm_arm.py              # 双臂驱动
│   ├── config_dm_arm.py            # 单臂配置
│   ├── config_dual_dm_arm.py       # 双臂配置
│   └── motors/
│       └── DM_Control_Python/      # 达妙电机 CAN 通信库
├── scripts/                        # 实用工具脚本
│   ├── teleop.py                   # 单臂遥操作
│   ├── dual_teleop.py              # 双臂遥操作
│   ├── infer_dm.py                 # 策略推理
│   ├── process_data.py             # 数据处理
│   └── ...
├── dataset/                        # 本地数据集存储
│   ├── data/                       # Parquet 数据文件
│   └── meta/                       # 统计信息
├── outputs/                        # 训练输出
│   ├── train/                      # 训练检查点
│   └── eval/                       # 评估结果
├── test/                           # 单元测试
├── pyproject.toml                  # 项目配置
└── Note.md                         # 详细技术文档
```

## 📚 高级用法

### 自定义硬件配置

编辑 `config_dm_arm.py`:
```python
@dataclass
class DMFollowerConfig(RobotConfig):
    port: str                           			# 通信端口
    disable_torque_on_disconnect: bool = True  		# 断开时禁用转矩
    joint_velocity_scaling: float = 0.2        		# 速度缩放因子
    max_gripper_torque: float = 1.0           		# 最大夹爪力矩
    cameras: dict[str, CameraConfig] = field(default_factory=dict)
```

### 使用 Hugging Face Hub

```bash
# 登录 HF
hf auth login
# 或指定 token
export HF_TOKEN=your_token_here

# 查询现有数据集
huggingface-cli repo list

# 下载模型推理
python scripts/infer_dm.py \
    --model_path my_username/my_act_model \
    --from_hub
```

### 数据集管理

```bash
# 配置 HF 缓存位置（避免卡满本地磁盘）
export HF_HOME="/media/$USER/AgroTech/home/huggingface"

# 查看已下载数据集
ls -lh $HF_HOME/lerobot/

# 删除不需要的数据集
rm -rf $HF_HOME/lerobot/<repo_id>
```

## 🔧 故障排查

### 串口连接问题

```bash
# 诊断所有 USB 设备
lsusb

# 查看串口权限
ls -l /dev/ttyACM* /dev/ttyUSB*

# 确认分配的符号链接
ls -l /dev/com-*

# 测试通信
python -c "import serial; s=serial.Serial('/dev/ttyACM0', 115200); print(s)"
```

### 摄像头问题

```bash
# 列出所有摄像头
v4l2-ctl --list-devices

# 检查特定摄像头能力
v4l2-ctl -d /dev/video0 --list-formats-ext

# 测试采集
python -c "import cv2; cap=cv2.VideoCapture(0); print(cap.isOpened())"
```

### 模型加载问题

```bash
# 验证模型结构
python -c "
from lerobot.policies import make_policy
policy = make_policy.from_pretrained('outputs/<model>/checkpoints/last/pretrained_model')
print(policy)
"

# 检查数据集可访问性
python -c "
from lerobot.datasets import LeRobotDataset
dataset = LeRobotDataset.from_huggingface('$USER/my_dataset')
print(f'Episodes: {len(dataset)}')
"
```

## 📖 参考文献

- **LeRobot**: [官方文档](https://huggingface.co/docs/lerobot)
- **SmolVLA**: [LeRobot SmolVLA](https://huggingface.co/lerobot/smolvla_base)
- **原始项目**: [TRLC-DK1](https://github.com/robot-learning-co/trlc-dk1)

## 📝 许可证

本项目采用 **MIT License** 开源，详见 [LICENSE](LICENSE) 文件。

## 👥 贡献

欢迎提交 Issues 和 Pull Requests！

## 📬 联系方式

如有问题或建议，请通过以下方式联系：
- GitHub Issues
- 提交讨论 (GitHub Discussions)
