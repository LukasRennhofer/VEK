#!/usr/bin/env bash
# ===============================================
# VEK Sample Builder
# Usage:
#   ./build.sh <sample_name> [--clean] [--backend opengl|vulkan]
# ===============================================

set -e

# Root paths
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VEK_DIR="$SCRIPT_DIR/../VEK"
BUILD_ROOT="$SCRIPT_DIR/../Build"

# Parse arguments
SAMPLE_NAME="$1"
CLEAN_BUILD=false
BACKEND="opengl" # default backend

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
    echo "Usage: ./build.sh <sample_name> [--clean] [--backend opengl|vulkan]"
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
BUILD_DIR="$BUILD_ROOT/$SAMPLE_NAME"
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
# Configure and build
# =========================
echo "Building sample '$SAMPLE_NAME' with backend '$BACKEND'..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Use platform-appropriate generator
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
    GENERATOR=(-G "Unix Makefiles")
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    GENERATOR=(-G "MinGW Makefiles")
else
    GENERATOR=""
fi

cmake "${GENERATOR[@]}" "$SAMPLE_DIR" \
    -DVEK_DIR="$VEK_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    $BACKEND_FLAGS

cmake --build . -- -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu || echo 4)"

# =========================
# Run executable if built
# =========================
EXEC_PATH="$(find . -maxdepth 1 -type f -executable ! -name 'cmake*' | head -n 1)"
if [ -f "$EXEC_PATH" ]; then
    echo "Build complete. Running sample..."
    "$EXEC_PATH"
else
    echo "Build complete. No executable found in $BUILD_DIR"
fi
