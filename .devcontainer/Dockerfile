# 使用微软提供的 Python 3.10 基础镜像
FROM mcr.microsoft.com/devcontainers/python:3.10

# 系统依赖：ffmpeg、串口、摄像头
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    cmake build-essential \
    libavformat-dev libavcodec-dev libswscale-dev \
    libavdevice-dev libavfilter-dev pkg-config \
    v4l-utils \
    udev \
    libusb-1.0-0-dev \
    && apt-get -y autoremove && rm -rf /var/lib/apt/lists/*

# 升级 pip 并安装基础构建工具
RUN pip install --upgrade pip hatchling

# 把容器内 vscode 用户加入 dialout/video 组
RUN usermod -aG dialout,video vscode