#!/bin/bash
set -euo pipefail

# ─── Resolve project root (works from any directory) ─────────────────────────
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BINARY="$BUILD_DIR/DTOME"

# ─── Parse arguments ─────────────────────────────────────────────────────────
BUILD_TYPE="Debug"
RUN_AFTER_BUILD=true
RUN_TESTS=false
CLEAN=true

usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "  -r, --release     Build in Release mode (optimised, no sanitisers)"
    echo "  -d, --debug       Build in Debug mode with ASan/UBSan (default)"
    echo "  -t, --tests       Build and run CTest unit tests"
    echo "  -n, --no-run      Build only, do not run the binary"
    echo "  -k, --keep-build  Skip cleaning the build directory"
    echo "  -h, --help        Show this help"
    echo ""
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -r|--release)  BUILD_TYPE="Release" ;;
        -d|--debug)    BUILD_TYPE="Debug"   ;;
        -t|--tests)    RUN_TESTS=true       ;;
        -n|--no-run)   RUN_AFTER_BUILD=false ;;
        -k|--keep-build) CLEAN=false        ;;
        -h|--help)     usage; exit 0        ;;
        *) echo "Unknown option: $1"; usage; exit 1 ;;
    esac
    shift
done

# ─── Colours ─────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'; RESET='\033[0m'
info()    { echo -e "${CYAN}[build]${RESET} $*"; }
success() { echo -e "${GREEN}[build]${RESET} $*"; }
error()   { echo -e "${RED}[build]${RESET} $*" >&2; }

# ─── Load .env ───────────────────────────────────────────────────────────────
ENV_FILE="$PROJECT_ROOT/.env"
if [[ -f "$ENV_FILE" ]]; then
    info "Loading environment from .env..."
    set -a
    # shellcheck disable=SC1090
    source "$ENV_FILE"
    set +a
else
    error ".env file not found at $ENV_FILE"
    error "Create it with: echo 'DTOME_DB_URL=postgresql://...' > .env"
    exit 1
fi

# ─── Validate required env vars ──────────────────────────────────────────────
if [[ -z "${DTOME_DB_URL:-}" ]]; then
    error "DTOME_DB_URL is not set in .env"
    exit 1
fi
info "DB URL       : ${DTOME_DB_URL:0:40}..."   # print first 40 chars only

# ─── Check cmake is available ────────────────────────────────────────────────
if ! command -v cmake &>/dev/null; then
    error "cmake not found. Install with: sudo apt install cmake"
    exit 1
fi

info "Project root : $PROJECT_ROOT"
info "Build type   : $BUILD_TYPE"

# ─── Clean ───────────────────────────────────────────────────────────────────
if $CLEAN; then
    info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ─── Configure ───────────────────────────────────────────────────────────────
info "Running CMake configure..."
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# compile_commands.json symlink at root (useful for clangd / IDE tooling)
if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
    ln -sf "$BUILD_DIR/compile_commands.json" "$PROJECT_ROOT/compile_commands.json"
fi

# ─── Build ───────────────────────────────────────────────────────────────────
info "Building..."
NPROC=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
cmake --build "$BUILD_DIR" -- -j"$NPROC"

if [[ ! -f "$BINARY" ]]; then
    error "Build succeeded but binary not found at $BINARY"
    exit 1
fi

success "Build complete → $BINARY"

# ─── Tests ───────────────────────────────────────────────────────────────────
if $RUN_TESTS; then
    info "Running CTest..."
    cd "$BUILD_DIR"
    ctest --output-on-failure
fi

# ─── Run ─────────────────────────────────────────────────────────────────────
if $RUN_AFTER_BUILD; then
    info "Running $BINARY..."
    echo "──────────────────────────────────────────"
    "$BINARY"
fi