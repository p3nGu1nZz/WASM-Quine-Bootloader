#!/usr/bin/env bash
# setup.sh – Verify and install all dependencies needed to build
#            WASM Quine Bootloader (C++/SDL3) on Windows WSL Ubuntu.
#
# Usage: bash setup.sh
# Run from the repository root.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXTERNAL_DIR="${REPO_ROOT}/external"

GREEN='\033[0;32m'; YELLOW='\033[1;33m'; RED='\033[0;31m'; NC='\033[0m'
info()    { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }
section() { echo -e "\n${GREEN}===== $* =====${NC}"; }

# ── 0. Root check ──────────────────────────────────────────────────────────────
if [[ "$EUID" -eq 0 ]]; then
    SUDO=""
else
    SUDO="sudo"
fi

# ── 1. System packages ─────────────────────────────────────────────────────────
section "System packages"

PACKAGES=(
    build-essential
    cmake
    ninja-build
    git
    pkg-config
    python3
    # SDL3 build dependencies
    libx11-dev
    libxext-dev
    libxrandr-dev
    libxcursor-dev
    libxi-dev
    libxinerama-dev
    libxss-dev
    libxxf86vm-dev
    libgl1-mesa-dev
    libgles2-mesa-dev
    libegl1-mesa-dev
    libwayland-dev
    libwayland-egl-backend-dev
    libxkbcommon-dev
    libasound2-dev
    libpulse-dev
    libdbus-1-dev
    libudev-dev
    libdrm-dev
    libgbm-dev
    libsndfile1-dev
    # Font for the UI (DejaVu monospace)
    fonts-dejavu-core
)

$SUDO apt-get update -qq
$SUDO apt-get install -y --no-install-recommends "${PACKAGES[@]}"
info "System packages installed."

# ── 2. Create external/ directory ─────────────────────────────────────────────
section "External libraries directory"
mkdir -p "${EXTERNAL_DIR}"
info "Directory: ${EXTERNAL_DIR}"

# ── Helper: clone or update a git repo ────────────────────────────────────────
clone_or_update() {
    local url="$1"
    local dir="$2"
    local branch="${3:-}"
    if [[ -d "${dir}/.git" ]]; then
        info "Updating $(basename "${dir}") …"
        git -C "${dir}" fetch --quiet
        if [[ -n "${branch}" ]]; then
            git -C "${dir}" checkout --quiet "${branch}"
        fi
        git -C "${dir}" pull --quiet
    else
        info "Cloning $(basename "${dir}") …"
        local args=(--depth 1)
        if [[ -n "${branch}" ]]; then
            args+=(--branch "${branch}")
        fi
        git clone --quiet "${args[@]}" "${url}" "${dir}"
    fi
}

# ── 3. wasm3 ──────────────────────────────────────────────────────────────────
section "wasm3 (WebAssembly interpreter)"
WASM3_DIR="${EXTERNAL_DIR}/wasm3"
clone_or_update "https://github.com/wasm3/wasm3.git" "${WASM3_DIR}" "main"
info "wasm3 ready at ${WASM3_DIR}"

# ── 4. ImGui ──────────────────────────────────────────────────────────────────
section "Dear ImGui"
IMGUI_DIR="${EXTERNAL_DIR}/imgui"
clone_or_update "https://github.com/ocornut/imgui.git" "${IMGUI_DIR}" "master"
info "ImGui ready at ${IMGUI_DIR}"

# ── 5. SDL3 ───────────────────────────────────────────────────────────────────
section "SDL3"
SDL3_SYS_VERSION=""

# Check if SDL3 is already installed system-wide
if pkg-config --exists sdl3 2>/dev/null; then
    SDL3_SYS_VERSION=$(pkg-config --modversion sdl3)
    info "SDL3 ${SDL3_SYS_VERSION} found via pkg-config (system install)."
elif [[ -f "${EXTERNAL_DIR}/SDL3/build/lib/cmake/SDL3/SDL3Config.cmake" ]]; then
    info "SDL3 already built in external/SDL3."
else
    warn "SDL3 not found – building from source (this may take a few minutes)."
    SDL3_SRC="${EXTERNAL_DIR}/SDL3"
    clone_or_update "https://github.com/libsdl-org/SDL.git" "${SDL3_SRC}" "release-3.2.x"

    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
    mkdir -p "${SDL3_SRC}/build"
    cmake -S "${SDL3_SRC}" -B "${SDL3_SRC}/build" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DSDL_SHARED=OFF \
        -DSDL_STATIC=ON \
        -DSDL_TESTS=OFF \
        -DSDL_AUDIO=OFF \
        -DCMAKE_INSTALL_PREFIX="${SDL3_SRC}/build" \
        -Wno-dev --log-level=WARNING
    cmake --build "${SDL3_SRC}/build" --parallel "$(nproc)"
    cmake --install "${SDL3_SRC}/build"
    info "SDL3 built and installed to ${SDL3_SRC}/build"
fi

# ── 6. Verify all required files are present ──────────────────────────────────
section "Verification"

CHECKS=(
    "${EXTERNAL_DIR}/wasm3/source/wasm3.h"
    "${EXTERNAL_DIR}/wasm3/source/m3_env.h"
    "${EXTERNAL_DIR}/imgui/imgui.h"
    "${EXTERNAL_DIR}/imgui/backends/imgui_impl_sdl3.h"
    "${EXTERNAL_DIR}/imgui/backends/imgui_impl_sdlrenderer3.h"
)

ALL_OK=true
for f in "${CHECKS[@]}"; do
    if [[ -f "$f" ]]; then
        info "✔  $(basename "$f")"
    else
        warn "✘  Missing: $f"
        ALL_OK=false
    fi
done

# SDL3 header check
if pkg-config --exists sdl3 2>/dev/null; then
    info "✔  SDL3 headers (system)"
elif [[ -f "${EXTERNAL_DIR}/SDL3/build/include/SDL3/SDL.h" ]]; then
    info "✔  SDL3 headers (external/SDL3)"
else
    warn "✘  SDL3 headers not found"
    ALL_OK=false
fi

if [[ "$ALL_OK" == "false" ]]; then
    error "One or more required files are missing. Please check the output above."
fi

# ── 7. Configure and build the project ────────────────────────────────────────
section "Build"
BUILD_DIR="${REPO_ROOT}/build"
mkdir -p "${BUILD_DIR}"

cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -Wno-dev --log-level=WARNING

cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

info ""
info "Build successful!  Run the app with:"
info "  ${BUILD_DIR}/wasm_quine_bootloader"
info ""
info "Or use:  bash scripts/run.sh"
