#!/bin/bash
# 打包脚本 - 生成可分发的 DMG 文件
# 用法: ./package_app.sh

set -e

# ========== 配置 ==========
APP_NAME="AILoginSystem"
VERSION="1.0.0"
DMG_NAME="${APP_NAME}_${VERSION}"

# API Key (内测分发用)
DIFY_API_KEY="app-4oFxsxMqCp4EYv0t77scpGDA"

# Qt 路径 (根据你的安装位置修改)
QT_PATH="/opt/homebrew/opt/qt@6"
if [ ! -d "$QT_PATH" ]; then
    QT_PATH="/usr/local/opt/qt@6"
fi
if [ ! -d "$QT_PATH" ]; then
    # 尝试查找 Qt Creator 安装的 Qt
    QT_PATH="$HOME/Qt/6.7.0/macos"
fi

MACDEPLOYQT="$QT_PATH/bin/macdeployqt"

# ========== 颜色输出 ==========
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
echo_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
echo_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# ========== 检查依赖 ==========
echo_info "检查依赖..."

if [ ! -f "$MACDEPLOYQT" ]; then
    echo_error "找不到 macdeployqt，请设置正确的 QT_PATH"
    echo "当前路径: $QT_PATH"
    echo "请修改脚本中的 QT_PATH 变量"
    exit 1
fi

# ========== 构建 ==========
echo_info "构建 Release 版本..."

BUILD_DIR="build_release"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.ncpu)

cd ..

APP_PATH="$BUILD_DIR/${APP_NAME}.app"

if [ ! -d "$APP_PATH" ]; then
    echo_error "构建失败，找不到 $APP_PATH"
    exit 1
fi

echo_info "构建成功: $APP_PATH"

# ========== 嵌入 API Key ==========
echo_info "嵌入 API Key 到 Info.plist..."

PLIST_PATH="$APP_PATH/Contents/Info.plist"

# 添加 LSEnvironment 键来设置环境变量
/usr/libexec/PlistBuddy -c "Add :LSEnvironment dict" "$PLIST_PATH" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Add :LSEnvironment:DIFY_API_KEY string $DIFY_API_KEY" "$PLIST_PATH" 2>/dev/null || \
/usr/libexec/PlistBuddy -c "Set :LSEnvironment:DIFY_API_KEY $DIFY_API_KEY" "$PLIST_PATH"

echo_info "API Key 已嵌入"

# ========== 打包 Qt 依赖 ==========
echo_info "打包 Qt 依赖 (macdeployqt)..."

"$MACDEPLOYQT" "$APP_PATH" -always-overwrite

# ========== 创建 DMG ==========
echo_info "创建 DMG 文件..."

DMG_DIR="dist"
rm -rf "$DMG_DIR"
mkdir -p "$DMG_DIR"

# 创建临时目录用于 DMG 内容
DMG_TEMP="$DMG_DIR/dmg_temp"
mkdir -p "$DMG_TEMP"

# 复制 app 到临时目录
cp -R "$APP_PATH" "$DMG_TEMP/"

# 创建 Applications 链接
ln -s /Applications "$DMG_TEMP/Applications"

# 创建 DMG
hdiutil create -volname "$APP_NAME" \
    -srcfolder "$DMG_TEMP" \
    -ov -format UDZO \
    "$DMG_DIR/${DMG_NAME}.dmg"

# 清理临时目录
rm -rf "$DMG_TEMP"

echo ""
echo_info "========== 打包完成 =========="
echo_info "DMG 文件: $DMG_DIR/${DMG_NAME}.dmg"
echo ""
echo_warn "分发说明:"
echo "  1. 将 DMG 文件发送给用户"
echo "  2. 用户打开 DMG，将 App 拖到 Applications"
echo "  3. 首次打开时，右键点击 → 打开 → 确认打开"
echo "     (因为没有代码签名，macOS 会提示安全警告)"
echo ""
