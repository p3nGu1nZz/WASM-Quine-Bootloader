#!/usr/bin/env bash
# setup.sh – Verify and install all dependencies needed to build
#            WASM Quine Bootloader (C++/SDL3) on Windows WSL Ubuntu.
#
# Usage:
#   bash scripts/setup.sh              – Linux only (default)
#   bash scripts/setup.sh windows      – also set up Windows cross-compile (MinGW + SDL3-windows)
#
# Run from the repository root.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SETUP_WINDOWS=false
[[ "${1:-}" == "windows" ]] && SETUP_WINDOWS=true

EXTERNAL_DIR="${REPO_ROOT}/external"

GREEN='\033[0;32m'; YELLOW='\033[1;33m'; RED='\033[0;31m'; NC='\033[0m'
info()    { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }
section() { echo -e "\n${GREEN}===== $* =====${NC}"; }

# ── 0. Sudo helper ─────────────────────────────────────────────────────────────
if [[ "$EUID" -eq 0 ]]; then
    SUDO=""
else
    SUDO="sudo"
fi

# ── 1. System packages ─────────────────────────────────────────────────────────
section "System packages"

PACKAGES=(
    build-essential cmake ninja-build git pkg-config python3
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev
    libxinerama-dev libxss-dev libxxf86vm-dev
    libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev
    libwayland-dev libxkbcommon-dev
    libudev-dev libdbus-1-dev libdrm-dev libgbm-dev
    fonts-dejavu-core
)
$SUDO apt-get update -qq
$SUDO apt-get install -y --no-install-recommends --fix-missing "${PACKAGES[@]}"
info "System packages installed."

# ── 2. MinGW-w64 (optional, for Windows cross-compile) ────────────────────────
if [[ "$SETUP_WINDOWS" == "true" ]]; then
    section "MinGW-w64 (Windows cross-compile)"
    $SUDO apt-get install -y --no-install-recommends mingw-w64
    info "MinGW-w64 installed."
fi

# ── 3. Create external/ directory structure ────────────────────────────────────
section "External libraries directory"
mkdir -p "${EXTERNAL_DIR}/SDL3"
info "Directory: ${EXTERNAL_DIR}"

# ── Helper: clone or update a git repo ────────────────────────────────────────
clone_or_update() {
    local url="$1" dir="$2" branch="${3:-}"
    if [[ -d "${dir}/.git" ]]; then
        info "Updating $(basename "${dir}") …"
        git -C "${dir}" fetch --quiet
        [[ -n "${branch}" ]] && git -C "${dir}" checkout --quiet "${branch}"
        git -C "${dir}" pull --quiet
    else
        info "Cloning $(basename "${dir}") …"
        local args=(--depth 1)
        [[ -n "${branch}" ]] && args+=(--branch "${branch}")
        git clone --quiet "${args[@]}" "${url}" "${dir}"
    fi
}

# ── 4. wasm3 ──────────────────────────────────────────────────────────────────
section "wasm3 (WebAssembly interpreter)"
clone_or_update "https://github.com/wasm3/wasm3.git" "${EXTERNAL_DIR}/wasm3" "main"
info "wasm3 ready."

# ── 5. ImGui ──────────────────────────────────────────────────────────────────
section "Dear ImGui"
clone_or_update "https://github.com/ocornut/imgui.git" "${EXTERNAL_DIR}/imgui" "master"
info "ImGui ready."

# ── 6. SDL3 (Linux) ───────────────────────────────────────────────────────────
# Layout:  external/SDL3/src/   – source tree
#          external/SDL3/linux/ – installed headers + libs for Linux
section "SDL3 (Linux)"
SDL3_SRC="${EXTERNAL_DIR}/SDL3/src"
SDL3_LINUX="${EXTERNAL_DIR}/SDL3/linux"

build_sdl3_linux() {
    clone_or_update "https://github.com/libsdl-org/SDL.git" "${SDL3_SRC}" "release-3.2.x"
    local bld="${SDL3_SRC}/build-linux"
    mkdir -p "${bld}"
    cmake -S "${SDL3_SRC}" -B "${bld}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TESTS=OFF \
        -DSDL_AUDIO=OFF \
        -DCMAKE_INSTALL_PREFIX="${SDL3_LINUX}" \
        -Wno-dev --log-level=WARNING
    cmake --build "${bld}" --parallel "$(nproc)"
    cmake --install "${bld}"
    info "SDL3 (Linux) installed at ${SDL3_LINUX}"
}

if pkg-config --exists sdl3 2>/dev/null; then
    info "SDL3 found via pkg-config (system install) – skipping build."
elif [[ -f "${SDL3_LINUX}/lib/cmake/SDL3/SDL3Config.cmake" ]]; then
    info "SDL3 (Linux) already built."
else
    warn "SDL3 not found – building from source (this may take a few minutes)."
    build_sdl3_linux
fi

# ── 7. SDL3 (Windows cross-compile) ──────────────────────────────────────────
# Layout:  external/SDL3/src/     – shared source tree (reused from above)
#          external/SDL3/windows/ – installed headers + libs for Windows
if [[ "$SETUP_WINDOWS" == "true" ]]; then
    section "SDL3 (Windows cross-compile)"
    SDL3_WIN="${EXTERNAL_DIR}/SDL3/windows"
    if [[ -f "${SDL3_WIN}/lib/cmake/SDL3/SDL3Config.cmake" ]]; then
        info "SDL3 (Windows) already built."
    else
        warn "Building SDL3 for Windows via MinGW-w64 (this may take a few minutes)."
        [[ -d "${SDL3_SRC}/.git" ]] || \
            clone_or_update "https://github.com/libsdl-org/SDL.git" "${SDL3_SRC}" "release-3.2.x"
        bld_win="${SDL3_SRC}/build-windows"
        mkdir -p "${bld_win}"
        cmake -S "${SDL3_SRC}" -B "${bld_win}" -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_TOOLCHAIN_FILE="${REPO_ROOT}/cmake/toolchain-windows-x64.cmake" \
            -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TESTS=OFF \
            -DSDL_AUDIO=OFF \
            -DCMAKE_INSTALL_PREFIX="${SDL3_WIN}" \
            -Wno-dev --log-level=WARNING
        cmake --build "${bld_win}" --parallel "$(nproc)"
        cmake --install "${bld_win}"
        info "SDL3 (Windows) installed at ${SDL3_WIN}"
    fi
fi

# ── 8. Verify required files ──────────────────────────────────────────────────
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
    if [[ -f "$f" ]]; then info "✔  $(basename "$f")"
    else warn "✘  Missing: $f"; ALL_OK=false; fi
done

if pkg-config --exists sdl3 2>/dev/null; then
    info "✔  SDL3 headers (system)"
elif [[ -f "${SDL3_LINUX}/include/SDL3/SDL.h" ]]; then
    info "✔  SDL3 headers (external/SDL3/linux)"
else
    warn "✘  SDL3 headers not found"; ALL_OK=false
fi

[[ "$ALL_OK" == "false" ]] && error "One or more required files are missing."

# ── 9. Build (Linux-debug by default) ────────────────────────────────────────
section "Build (linux-debug)"
bash "${REPO_ROOT}/scripts/build.sh" linux-debug

info ""
info "Setup complete!  Run the app with:"
info "  bash scripts/run.sh"
info ""
info "Other build targets:"
info "  bash scripts/build.sh linux-release"
info "  bash scripts/build.sh windows-debug    # requires: bash scripts/setup.sh windows"
info "  bash scripts/build.sh windows-release  # requires: bash scripts/setup.sh windows"
