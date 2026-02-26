# 使用微软提供的 Python 3.10 基础镜像
FROM mcr.microsoft.com/devcontainers/python:3.10

# 安装系统依赖：
# - 编译类：cmake, build-essential
# - 视频/音频类：ffmpeg 相关的 libav 系列
# - OpenCV 图形类：libgl1, libglib2.0
# - 串口通讯类：libusb-1.0
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    cmake \
    build-essential \
    pkg-config \
    libavformat-dev \
    libavcodec-dev \
    libswscale-dev \
    libavdevice-dev \
    libavfilter-dev \
    libgl1-mesa-glx \
    libglib2.0-0 \
    libusb-1.0-0-dev \
    && rm -rf /var/lib/apt/lists/*

# 升级 pip 并安装基础构建工具
RUN pip install --upgrade pip hatchling