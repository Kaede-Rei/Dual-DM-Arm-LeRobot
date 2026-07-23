#!/bin/bash
# activate.sh
# 便携式 micromamba 激活脚本
# 使用方式:
#     . ./activate.sh
#     . ./activate.sh lerobot
#     source ./activate.sh lerobot

# 找到脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

OS_TYPE="$(uname -s)"
if [ "$OS_TYPE" != "Linux" ]; then
    echo " 错误: 此环境仅支持 Linux 系统"
    echo " 当前系统: $OS_TYPE"
    return 1 2>/dev/null || exit 1
fi

ARCH_TYPE="$(uname -m)"
REQUIRED_ARCH="x86_64"
if [ "$ARCH_TYPE" != "$REQUIRED_ARCH" ]; then
    echo " 警告: 架构不匹配！"
    echo " 需要: $REQUIRED_ARCH, 当前: $ARCH_TYPE"
    echo " 按任意键继续，或 Ctrl+C 退出..."
    read -n 1
fi

if grep -qi microsoft /proc/version 2>/dev/null; then
    echo " 检测到 WSL 环境；建议在原生 Linux 中运行以确保驱动和性能正常"
    # 如果想强制禁止 WSL，可以取消下面这一行的注释
    # return 1 2>/dev/null || exit 1
fi

# micromamba 主目录
MAMBA_ROOT="${SCRIPT_DIR}/micromamba"

# micromamba 可执行文件
MAMBA_BIN="${MAMBA_ROOT}/bin/micromamba"

if [ ! -x "$MAMBA_BIN" ]; then
    echo "错误:未找到 micromamba 可执行文件:"
    echo "      $MAMBA_BIN"
    echo "请确认已将 micromamba 解压到 micromamba/ 目录"
    return 1 2>/dev/null || exit 1
fi

# 目标环境(默认 base)
ENV_NAME="${1:-base}"
echo "目标环境: $ENV_NAME"

# 便携缓存目录
PORTABLE_CACHE="${SCRIPT_DIR}/.cache"
PORTABLE_PKGS="${PORTABLE_CACHE}/.pkgs"
PORTABLE_TMP="${PORTABLE_CACHE}/.tmp"
PORTABLE_PIP="${PORTABLE_CACHE}/.pip"
mkdir -p "$PORTABLE_CACHE" "$PORTABLE_PKGS" "$PORTABLE_TMP" "$PORTABLE_PIP"

# micromamba 专用环境变量
export MAMBA_ROOT_PREFIX="$MAMBA_ROOT"
export MAMBA_EXE="$MAMBA_BIN"

# 让 envs / pkgs / pip / tmp 全都放移动硬盘
export MAMBA_ENVS_DIRS="${MAMBA_ROOT}/envs"
export MAMBA_PKGS_DIRS="$PORTABLE_PKGS"
export PIP_CACHE_DIR="$PORTABLE_PIP"
export TMPDIR="$PORTABLE_TMP"
export TEMP="$PORTABLE_TMP"
export TMP="$PORTABLE_TMP"

# 完全禁用用户 site-packages
export PIP_USER=no
export PYTHONNOUSERSITE=1
export PYTHONUSERBASE="${SCRIPT_DIR}/.local"

# 清理系统里已有 conda/mamba 影响
unset CONDA_PREFIX
unset CONDA_DEFAULT_ENV
unset CONDA_SHLVL
unset PYTHONPATH

# 保存系统核心路径
SYSTEM_CORE_PATHS="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# 清理 PATH 中的 conda/mamba 相关路径
CLEANED_PATH=$(echo "$PATH" | tr ':' '\n' | grep -v -E "conda|mamba|\.local/bin" | tr '\n' ':' | sed 's/:$//')

# 确保系统核心路径存在
if ! echo "$CLEANED_PATH" | grep -q "/usr/bin"; then
    CLEANED_PATH="${CLEANED_PATH}:${SYSTEM_CORE_PATHS}"
fi

# 设置新的 PATH
export PATH="${MAMBA_ROOT}/bin:${CLEANED_PATH}"

# 初始化 shell hook
eval "$($MAMBA_BIN shell hook -s bash)"

# 创建环境(如果不存在)
if ! micromamba env list | awk '{print $1}' | grep -qx "$ENV_NAME"; then
    if [ "$ENV_NAME" != "base" ]; then
        echo "环境 $ENV_NAME 不存在,是否创建?(y/n)"
        read -r ans
        if [ "$ans" = "y" ]; then
            micromamba create -y -n "$ENV_NAME" python=3.10
        else
            return 1
        fi
    fi
fi

# 激活
micromamba activate "$ENV_NAME"

if [ $? -eq 0 ]; then
    ENV_PATH="${MAMBA_ENVS_DIRS}/${ENV_NAME}"
    FIXED_ISSUES=0
    
    echo ""
    echo "========================================"
    echo "执行深度修复检查..."
    echo "========================================"
    
    # 修复可执行文件的 shebang
    echo ""
    echo "[1/4] 检查可执行文件 shebang..."
    
    BIN_DIR="${ENV_PATH}/bin"
    if [ -d "$BIN_DIR" ]; then
        CORRECT_PYTHON="${ENV_PATH}/bin/python3.10"
        
        # 只检查已知的 Python 脚本,避免扫描二进制文件
        for exe_file in "$BIN_DIR"/{pip,pip3,wheel,easy_install,easy_install-*}; do
            if [ -f "$exe_file" ]; then
                # 使用 file 命令检查是否是文本文件
                if file "$exe_file" | grep -q "text\|script"; then
                    first_line=$(head -n 1 "$exe_file" 2>/dev/null)
                    
                    if [[ "$first_line" == "#!"*"python"* ]]; then
                        current_shebang=$(echo "$first_line" | sed 's/^#!//')
                        
                        # 检查是否需要修复
                        if [[ "$current_shebang" == *"/media/kaeret/"* ]] || \
                           [[ "$current_shebang" == *"/media/zyt/"* && "$current_shebang" != "$CORRECT_PYTHON" ]] || \
                           [[ "$current_shebang" != "$CORRECT_PYTHON" && "$current_shebang" != "${ENV_PATH}/bin/python"* ]]; then
                            
                            echo "  [修复] $(basename $exe_file)"
                            echo "    旧: $first_line"
                            
                            # 备份并修复
                            cp "$exe_file" "${exe_file}.bak"
                            tail -n +2 "$exe_file" > "${exe_file}.tmp"
                            echo "#!${CORRECT_PYTHON}" > "${exe_file}.new"
                            cat "${exe_file}.tmp" >> "${exe_file}.new"
                            mv "${exe_file}.new" "$exe_file"
                            chmod +x "$exe_file"
                            rm -f "${exe_file}.tmp"
                            
                            echo "    新: #!${CORRECT_PYTHON}"
                            FIXED_ISSUES=$((FIXED_ISSUES + 1))
                        fi
                    fi
                fi
            fi
        done
        
        if [ $FIXED_ISSUES -eq 0 ]; then
            echo "  未发现问题"
        fi
    fi
    
    # 清理 .pth 文件
    echo ""
    echo "[2/4] 检查 .pth 文件..."
    
    PTH_FIXED=0
    if [ -d "${ENV_PATH}/lib" ]; then
        find "${ENV_PATH}/lib" -name "*.pth" -type f 2>/dev/null | while read pth_file; do
            # 检查是否包含问题路径
            if grep -q "/media/kaeret\|/media/zyt.*lerobot\|/home/.*/lerobot" "$pth_file" 2>/dev/null; then
                echo "  [发现] $(basename $pth_file)"
                
                # 如果是 lerobot 相关,直接删除
                if grep -q "lerobot\|robot.*multi" "$pth_file" 2>/dev/null; then
                    echo "    包含旧路径,已删除"
                    rm -f "$pth_file"
                else
                    # 其他文件,清理旧路径
                    cp "$pth_file" "${pth_file}.bak"
                    grep -v "/media/kaeret\|/media/zyt.*lerobot\|/home/.*/lerobot" "$pth_file" > "${pth_file}.tmp"
                    
                    if [ -s "${pth_file}.tmp" ]; then
                        mv "${pth_file}.tmp" "$pth_file"
                        echo "    已清理旧路径"
                    else
                        rm -f "$pth_file" "${pth_file}.tmp"
                        echo "    清理后为空,已删除"
                    fi
                fi
                PTH_FIXED=$((PTH_FIXED + 1))
            fi
        done
        
        if [ $PTH_FIXED -eq 0 ]; then
            echo "  未发现问题"
        else
            FIXED_ISSUES=$((FIXED_ISSUES + PTH_FIXED))
        fi
    fi
    
    # 清理旧的包安装
    echo ""
    echo "[3/4] 检查旧的包安装..."
    
    PKG_FIXED=0
    SITE_PACKAGES="${ENV_PATH}/lib/python*/site-packages"
    for sp_dir in $SITE_PACKAGES; do
        if [ -d "$sp_dir" ]; then
            # 删除包含旧路径的目录
            find "$sp_dir" -maxdepth 1 \( -name "*lerobot*" -o -name "*robot*.egg-info" -o -name "__editable__*" \) -type d 2>/dev/null | while read old_dir; do
                if grep -r "/media/kaeret" "$old_dir" 2>/dev/null | grep -q .; then
                    echo "  [清理] $(basename $old_dir)"
                    rm -rf "$old_dir"
                    PKG_FIXED=$((PKG_FIXED + 1))
                fi
            done
            
            # 清理 easy-install.pth
            EI_PTH="$sp_dir/easy-install.pth"
            if [ -f "$EI_PTH" ]; then
                if grep -q "/media/kaeret" "$EI_PTH" 2>/dev/null; then
                    echo "  [修复] easy-install.pth"
                    cp "$EI_PTH" "${EI_PTH}.bak"
                    grep -v "/media/kaeret" "$EI_PTH" > "${EI_PTH}.tmp"
                    mv "${EI_PTH}.tmp" "$EI_PTH"
                    PKG_FIXED=$((PKG_FIXED + 1))
                fi
            fi
        fi
    done
    
    if [ $PKG_FIXED -eq 0 ]; then
        echo "  未发现问题"
    else
        FIXED_ISSUES=$((FIXED_ISSUES + PKG_FIXED))
    fi
    
    # 验证环境
    echo ""
    echo "[4/4] 验证环境..."
    
    if ! python --version &>/dev/null; then
        echo "  Python 不可用"
    else
        echo "  Python: $(python --version)"
    fi
    
    if ! python -m pip --version &>/dev/null; then
        echo "  pip 不可用"
    else
        echo "  pip: $(python -m pip --version | head -n 1)"
    fi
    
    echo ""
    echo "========================================"
    if [ $FIXED_ISSUES -gt 0 ]; then
        echo "已修复 $FIXED_ISSUES 个问题"
    else
        echo "未发现问题"
    fi
    echo "========================================"
    
    # 强制重新设置环境变量
    export PYTHONNOUSERSITE=1
    export PYTHONUSERBASE="${SCRIPT_DIR}/.local"
    
    echo ""
    echo "--------------------------------------------------"
    echo "- 成功激活: ${ENV_NAME}"
    echo "- micromamba: $(which micromamba)"
    echo "- python:     $(which python)"
    echo "- pip:        $(which pip)"
    echo "- envs:       $MAMBA_ENVS_DIRS"
    echo "- pkgs:       $MAMBA_PKGS_DIRS"
    echo "- pip cache:  $PIP_CACHE_DIR"
    echo "- tmp:        $TMPDIR"
    echo "--------------------------------------------------"
    
    # 处理开发包安装
    WORKSPACE_DIR="${SCRIPT_DIR}/LeRobot-Workspace/custom-hw-sim"
    if [ -f "${WORKSPACE_DIR}/pyproject.toml" ]; then
        echo ""
        echo "检测到工作区: ${WORKSPACE_DIR}"
        
        NEEDS_INSTALL=0
        
        # 检查包是否已安装
        if ! python -m pip show lerobot &>/dev/null; then
            echo "  状态: 未安装"
            NEEDS_INSTALL=1
        else
            # 检查是否可编辑安装
            EDITABLE_CHECK=$(python -m pip list --editable 2>/dev/null | grep -i lerobot)
            
            if [ -z "$EDITABLE_CHECK" ]; then
                echo "  状态: 已安装(非可编辑模式)"
                echo "  提示: 建议使用可编辑模式安装以便开发"
                NEEDS_INSTALL=1
            else
                # 已安装且为可编辑模式,检查是否能正常导入
                if python -c "import lerobot" 2>/dev/null; then
                    echo "  状态: 已正确安装(可编辑模式)"
                    
                    # 尝试获取版本信息
                    VERSION=$(python -c "import lerobot; print(lerobot.__version__)" 2>/dev/null)
                    if [ -n "$VERSION" ]; then
                        echo "  版本: $VERSION"
                    fi
                    
                    # 显示源码位置(如果能找到)
                    SOURCE_FILE=$(python -c "import lerobot, os; print(os.path.realpath(lerobot.__file__))" 2>/dev/null)
                    if [ -n "$SOURCE_FILE" ]; then
                        SOURCE_DIR=$(dirname $(dirname "$SOURCE_FILE"))
                        # 如果源码在工作区内,显示相对路径
                        if [[ "$SOURCE_DIR" == "${WORKSPACE_DIR}"* ]]; then
                            echo "  源码: ${WORKSPACE_DIR}"
                        fi
                    fi
                else
                    echo "  状态: 已安装但无法导入"
                    NEEDS_INSTALL=1
                fi
            fi
        fi
        
        # 只在需要时才提示安装
        if [ $NEEDS_INSTALL -eq 1 ]; then
            echo ""
            if [ $FIXED_ISSUES -gt 0 ]; then
                echo "  检测到环境修复,建议重新安装开发包"
                echo "是否安装/重新安装开发包?(y/n)"
            else
                echo "是否安装开发包?(y/n)"
            fi
            read -r install_ans
            
            if [ "$install_ans" = "y" ]; then
                echo ""
                echo "正在安装开发包..."
                echo "命令: python -m pip install -e ${WORKSPACE_DIR}"
                
                python -m pip install -e "${WORKSPACE_DIR}" --force-reinstall --no-deps
                
                if [ $? -eq 0 ]; then
                    echo ""
                    echo "开发包安装成功"
                    
                    NEW_PATH=$(python -c "import lerobot, os; print(os.path.dirname(lerobot.__file__))" 2>/dev/null)
                    if [ -n "$NEW_PATH" ]; then
                        echo "  位置: $NEW_PATH"
                        echo "  导入测试成功"
                    fi
                else
                    echo ""
                    echo "安装失败"
                fi
            fi
        fi
    fi
    
    echo ""
    echo "提示:如果需要诊断,请运行:"
    echo "  python -c \"import sys; print('\\n'.join(sys.path))\""
    echo "  python -c \"import lerobot; print(lerobot.__file__)\""
    echo ""
    
else
    echo "环境激活失败"
    micromamba env list
fi
