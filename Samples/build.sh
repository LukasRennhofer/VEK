#!/usr/bin/env bash
# ===============================================
# VEK Sample Builder (Cross-Platform)
# Usage:
#   ./build.sh <sample_name> [--clean] [--backend opengl|vulkan] [--platform linux|windows]
# ===============================================

set -e

# Root paths
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VEK_DIR="$SCRIPT_DIR/../VEK"
BUILD_ROOT="$SCRIPT_DIR/../Build"
TOOLCHAIN_FILE="$SCRIPT_DIR/toolchain-mingw64.cmake"

# Parse arguments
SAMPLE_NAME="$1"
CLEAN_BUILD=false
BACKEND="opengl"  # default backend
PLATFORM="linux"  # default platform

shift || true
while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)
            CLEAN_BUILD=true
            ;;
        --backend)
            shift
            BACKEND="$1"
            ;;
        --platform)
            shift
            PLATFORM="$1"
            ;;
        *)
            echo "Unknown option: $1"
            ;;
    esac
    shift
done

# =========================
# Check input validity
# =========================
if [ -z "$SAMPLE_NAME" ]; then
    echo "No sample specified."
    echo "Usage: ./build.sh <sample_name> [--clean] [--backend opengl|vulkan] [--platform linux|windows]"
    echo
    echo "Available samples:"
    for dir in "$SCRIPT_DIR"/*/; do
        [ -f "${dir}CMakeLists.txt" ] && echo "  - $(basename "$dir")"
    done
    exit 1
fi

SAMPLE_DIR="$SCRIPT_DIR/$SAMPLE_NAME"
if [ ! -d "$SAMPLE_DIR" ] || [ ! -f "$SAMPLE_DIR/CMakeLists.txt" ]; then
    echo "Sample '$SAMPLE_NAME' not found!"
    exit 1
fi

# =========================
# Handle clean builds
# =========================
BUILD_DIR="$BUILD_ROOT/${SAMPLE_NAME}-${PLATFORM}"
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning previous build at $BUILD_DIR ..."
    rm -rf "$BUILD_DIR"
fi

# =========================
# Configure backend
# =========================
if [ "$BACKEND" == "opengl" ]; then
    BACKEND_FLAGS="-DVEK_USE_OPENGL=ON -DVEK_USE_VULKAN=OFF"
elif [ "$BACKEND" == "vulkan" ]; then
    BACKEND_FLAGS="-DVEK_USE_OPENGL=OFF -DVEK_USE_VULKAN=ON"
else
    echo "Invalid backend: '$BACKEND'. Must be 'opengl' or 'vulkan'."
    exit 1
fi

# =========================
# Configure platform
# =========================
case "$PLATFORM" in
    linux)
        GENERATOR=(-G "Unix Makefiles")
        PLATFORM_FLAGS=""
        ;;
    windows)
        if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
            echo "❌ MinGW-w64 toolchain not found. Install with:"
            echo "   sudo pacman -S mingw-w64-gcc mingw-w64-binutils"
            exit 1
        fi
        GENERATOR=(-G "Unix Makefiles")
        PLATFORM_FLAGS="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE"
        ;;
    *)
        echo "Invalid platform: '$PLATFORM'. Must be 'linux' or 'windows'."
        exit 1
        ;;
esac

# =========================
# Configure and build
# =========================
echo "Building sample '$SAMPLE_NAME' for platform '$PLATFORM' with backend '$BACKEND'..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "${GENERATOR[@]}" "$SAMPLE_DIR" \
    -DVEK_DIR="$VEK_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    $BACKEND_FLAGS \
    $PLATFORM_FLAGS

cmake --build . -- -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu || echo 4)"

# =========================
# Run executable if built
# =========================
EXEC_PATH="$(find . -maxdepth 1 -type f -executable ! -name 'cmake*' | head -n 1)"

if [ "$PLATFORM" == "windows" ]; then
    EXE_PATH="$(find . -maxdepth 1 -type f -name '*.exe' | head -n 1)"
    if [ -f "$EXE_PATH" ]; then
        echo "Build complete. Windows executable: $EXE_PATH"
        if command -v wine >/dev/null 2>&1; then
            echo "Running under Wine..."
            wine "$EXE_PATH"
        fi
    else
        echo "Build complete. No Windows executable found in $BUILD_DIR"
    fi
else
    if [ -f "$EXEC_PATH" ]; then
        echo "Build complete. Running sample..."
        "$EXEC_PATH"
    else
        echo "Build complete. No executable found in $BUILD_DIR"
    fi
fi
