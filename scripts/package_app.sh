#!/bin/bash
# 打包脚本 - 生成可分发的 macOS App 和 DMG 文件
# 用法:
#   ./scripts/package_app.sh
#   ./scripts/package_app.sh --version 1.0.0 --arch-label arm64 --embed-release-keys

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
APP_NAME="AILoginSystem"
PRODUCT_NAME="AI思政智慧课堂"

VERSION="${VERSION:-}"
QT_PATH="${QT_PATH:-${QT_ROOT_DIR:-}}"
BUILD_DIR="${BUILD_DIR:-build_release}"
OUTPUT_DIR="${OUTPUT_DIR:-dist}"
ARCH_LABEL="${ARCH_LABEL:-}"
EMBED_RELEASE_KEYS="${EMBED_RELEASE_KEYS:-0}"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
echo_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
echo_error() { echo -e "${RED}[ERROR]${NC} $1"; }

usage() {
    cat <<'EOF'
用法: ./scripts/package_app.sh [options]

Options:
  --version <version>         指定版本号（支持 vX.Y.Z，默认读取 CMakeLists.txt）
  --qt-path <path>            指定 Qt 安装根目录
  --build-dir <path>          指定构建目录（默认: build_release）
  --output-dir <path>         指定产物目录（默认: dist）
  --arch-label <label>        指定产物架构标签（默认按当前机器推断）
  --embed-release-keys        生成发布版 embedded_keys.h（从环境变量读取密钥）
  --no-embed-release-keys     不生成发布版 embedded_keys.h
  --help                      显示帮助
EOF
}

resolve_path() {
    local path_value="$1"
    if [[ "$path_value" = /* ]]; then
        printf '%s\n' "$path_value"
    else
        printf '%s\n' "$REPO_ROOT/$path_value"
    fi
}

get_project_version() {
    local version
    version="$(sed -nE 's/.*project\(AILoginSystem VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$REPO_ROOT/CMakeLists.txt")"
    if [[ -z "$version" ]]; then
        echo_error "无法从 CMakeLists.txt 读取项目版本号"
        exit 1
    fi

    printf '%s\n' "$version"
}

normalize_version() {
    local raw_version="$1"
    if [[ -z "$raw_version" ]]; then
        raw_version="$(get_project_version)"
    fi

    raw_version="${raw_version#v}"
    printf '%s\n' "$raw_version"
}

normalize_arch_label() {
    local raw_arch="$1"
    if [[ -z "$raw_arch" ]]; then
        raw_arch="$(uname -m)"
    fi

    case "$raw_arch" in
        arm64|aarch64)
            printf 'arm64\n'
            ;;
        x86_64|amd64)
            printf 'x64\n'
            ;;
        *)
            printf '%s\n' "$raw_arch"
            ;;
    esac
}

find_macdeployqt() {
    if [[ -n "$QT_PATH" && -x "$QT_PATH/bin/macdeployqt" ]]; then
        printf '%s\n' "$QT_PATH/bin/macdeployqt"
        return
    fi

    if command -v macdeployqt >/dev/null 2>&1; then
        command -v macdeployqt
        return
    fi

    local candidates=(
        "/opt/homebrew/opt/qt@6"
        "/usr/local/opt/qt@6"
        "$HOME/Qt/6.7.0/macos"
        "$HOME/Qt/6.6.2/macos"
        "$HOME/Qt/6.8.0/macos"
    )

    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -x "$candidate/bin/macdeployqt" ]]; then
            printf '%s\n' "$candidate/bin/macdeployqt"
            return
        fi
    done

    echo_error "找不到 macdeployqt，请设置正确的 QT_PATH 或将其加入 PATH"
    exit 1
}

escape_cpp_string() {
    local value="$1"
    value="${value//\\/\\\\}"
    value="${value//\"/\\\"}"
    value="${value//$'\r'/}"
    value="${value//$'\n'/}"
    printf '%s' "$value"
}

generate_config_env() {
    if [[ "$EMBED_RELEASE_KEYS" != "1" ]]; then
        echo_info "跳过密钥导出，不生成 config.env"
        return
    fi

    # config.env 放进 App Bundle 的 Resources 目录
    local config_env_path="$APP_PATH/Contents/Resources/config.env"

    cat > "$config_env_path" <<EOF
# AI 思政智慧课堂 - 运行时配置
# 此文件包含 API 密钥，请勿公开分享

DIFY_API_KEY=${DIFY_API_KEY:-}
MINIMAX_API_KEY=${MINIMAX_API_KEY:-}
MINIMAX_API_BASE_URL=${MINIMAX_API_BASE_URL:-https://api.minimaxi.com/v1}
MINIMAX_MODEL=${MINIMAX_MODEL:-MiniMax-M2.7}
TIANXING_API_KEY=${TIANXING_API_KEY:-}
SUPABASE_URL=${SUPABASE_URL:-}
SUPABASE_ANON_KEY=${SUPABASE_ANON_KEY:-}
ZHIPU_API_KEY=${ZHIPU_API_KEY:-}
ZHIPU_BASE_URL=${ZHIPU_BASE_URL:-https://api.minimaxi.com/v1}
EOF

    echo_info "已生成发布版 config.env"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --qt-path)
            QT_PATH="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --arch-label)
            ARCH_LABEL="$2"
            shift 2
            ;;
        --embed-release-keys)
            EMBED_RELEASE_KEYS="1"
            shift
            ;;
        --no-embed-release-keys)
            EMBED_RELEASE_KEYS="0"
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo_error "未知参数: $1"
            usage
            exit 1
            ;;
    esac
done

VERSION="$(normalize_version "$VERSION")"
ARCH_LABEL="$(normalize_arch_label "$ARCH_LABEL")"
BUILD_DIR="$(resolve_path "$BUILD_DIR")"
OUTPUT_DIR="$(resolve_path "$OUTPUT_DIR")"
MACDEPLOYQT="$(find_macdeployqt)"
APP_PATH="$BUILD_DIR/${APP_NAME}.app"
DMG_NAME="${PRODUCT_NAME}-macOS-${ARCH_LABEL}-${VERSION}.dmg"
DMG_PATH="$OUTPUT_DIR/$DMG_NAME"
DMG_TEMP="$OUTPUT_DIR/dmg_temp"

if [[ -d "$BUILD_DIR" ]]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
mkdir -p "$OUTPUT_DIR"

echo_info "版本号: $VERSION"
echo_info "构建目录: $BUILD_DIR"
echo_info "产物目录: $OUTPUT_DIR"
echo_info "架构标签: $ARCH_LABEL"
echo_info "macdeployqt: $MACDEPLOYQT"

echo_info "配置 CMake Release 构建..."
CMAKE_ARGS=(-S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release)
if [[ -n "$QT_PATH" ]]; then
    CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$QT_PATH/lib/cmake")
fi
if command -v ninja >/dev/null 2>&1; then
    CMAKE_ARGS+=(-G Ninja)
fi
cmake "${CMAKE_ARGS[@]}"

echo_info "编译应用..."
cmake --build "$BUILD_DIR" --config Release

if [[ ! -d "$APP_PATH" && -d "$BUILD_DIR/Release/${APP_NAME}.app" ]]; then
    APP_PATH="$BUILD_DIR/Release/${APP_NAME}.app"
fi

if [[ ! -d "$APP_PATH" ]]; then
    echo_error "构建失败，找不到 ${APP_NAME}.app"
    exit 1
fi

echo_info "构建成功: $APP_PATH"

echo_info "打包 Qt 依赖 (macdeployqt)..."
"$MACDEPLOYQT" "$APP_PATH" -always-overwrite

# 生成运行时配置文件（密钥随包分发）
generate_config_env

if [[ -e "$DMG_PATH" ]]; then
    rm -f "$DMG_PATH"
fi
rm -rf "$DMG_TEMP"
mkdir -p "$DMG_TEMP"

echo_info "准备 DMG 内容..."
cp -R "$APP_PATH" "$DMG_TEMP/"
ln -s /Applications "$DMG_TEMP/Applications"

echo_info "创建 DMG 文件..."
hdiutil create -volname "$PRODUCT_NAME" \
    -srcfolder "$DMG_TEMP" \
    -ov -format UDZO \
    "$DMG_PATH"

rm -rf "$DMG_TEMP"

echo ""
echo_info "========== 打包完成 =========="
echo_info "App Bundle: $APP_PATH"
echo_info "DMG 文件: $DMG_PATH"
echo ""
echo_warn "分发说明:"
echo "  1. 将 DMG 文件发送给用户"
echo "  2. 用户打开 DMG，将 App 拖到 Applications"
echo "  3. 首次打开时，右键点击 → 打开 → 确认打开"
echo "     (因为没有代码签名，macOS 会提示安全警告)"
echo ""
