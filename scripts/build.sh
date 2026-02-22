#!/usr/bin/env bash
# build.sh – Build the WASM Quine Bootloader.
#
# Usage:  bash scripts/build.sh [TARGET | --clean]
#
# Targets:
#   linux-debug     (default) – GCC/Clang, debug symbols
#   linux-release             – GCC/Clang, optimised
#   windows-debug             – MinGW-w64 cross-compile, debug symbols
#   windows-release           – MinGW-w64 cross-compile, optimised
#
# Special commands:
#   --clean         Remove the entire build/ directory and any cached data
#
# The binary is placed in:
#   build/<TARGET>/bootloader         (Linux)
#   build/<TARGET>/bootloader.exe     (Windows)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="${1:-linux-debug}"
JOBS="${JOBS:-$(nproc)}"

# ── Clean ─────────────────────────────────────────────────────────────────────
if [[ "${TARGET}" == "--clean" ]]; then
    echo "[build] Cleaning build directory and caches..."
    rm -rf "${REPO_ROOT}/build"
    # Remove CMake / Ninja caches that may live at the repo root
    rm -f  "${REPO_ROOT}/CMakeCache.txt"
    rm -rf "${REPO_ROOT}/CMakeFiles"
    rm -f  "${REPO_ROOT}/cmake_install.cmake"
    rm -f  "${REPO_ROOT}/compile_commands.json"
    rm -f  "${REPO_ROOT}/install_manifest.txt"
    # Remove any leftover telemetry, logs, or sequence directories that might
    # have been created during earlier runs or tests.
    rm -rf "${REPO_ROOT}/bin"
    # Remove telemetry exports left by the binary (old behaviour)
    rm -f  "${REPO_ROOT}"/quine_telemetry_*.txt
    echo "[build] Clean done."
    exit 0
fi
# ── Resolve build type & platform ─────────────────────────────────────────────
case "${TARGET}" in
    linux-debug)    BUILD_TYPE=Debug;   PLATFORM=Linux   ;;
    linux-release)  BUILD_TYPE=Release; PLATFORM=Linux   ;;
    windows-debug)  BUILD_TYPE=Debug;   PLATFORM=Windows ;;
    windows-release)BUILD_TYPE=Release; PLATFORM=Windows ;;
    *)
        echo "[build] ERROR: unknown target '${TARGET}'."
        echo "[build] Valid targets: linux-debug linux-release windows-debug windows-release"
        exit 1
        ;;
esac

BUILD_DIR="${REPO_ROOT}/build/${TARGET}"
TOOLCHAIN_ARG=""

# ── Linux: use external/SDL3/linux if present (no system SDL3) ────────────────
if [[ "${PLATFORM}" == "Linux" ]]; then
    SDL3_LIN="${REPO_ROOT}/external/SDL3/linux"
    if [[ -f "${SDL3_LIN}/lib/cmake/SDL3/SDL3Config.cmake" ]]; then
        TOOLCHAIN_ARG="-DSDL3_DIR=${SDL3_LIN}/lib/cmake/SDL3"
    fi
fi

# ── Windows: MinGW-w64 toolchain ──────────────────────────────────────────────
if [[ "${PLATFORM}" == "Windows" ]]; then
    TOOLCHAIN_FILE="${REPO_ROOT}/cmake/toolchain-windows-x64.cmake"
    if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
        echo "[build] ERROR: toolchain file not found: ${TOOLCHAIN_FILE}"
        exit 1
    fi
    if ! command -v x86_64-w64-mingw32-gcc &>/dev/null; then
        echo "[build] ERROR: MinGW-w64 not found."
        echo "[build] Install with: sudo apt-get install mingw-w64"
        exit 1
    fi
    TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"

    # Cross-compiled SDL3 lives in external/SDL3/windows/
    SDL3_WIN="${REPO_ROOT}/external/SDL3/windows"
    if [[ ! -f "${SDL3_WIN}/lib/cmake/SDL3/SDL3Config.cmake" ]]; then
        echo "[build] Windows SDL3 not found at ${SDL3_WIN}."
        echo "[build] Run: bash scripts/setup.sh windows  (to build SDL3 for Windows)"
        exit 1
    fi
    TOOLCHAIN_ARG="${TOOLCHAIN_ARG} -DSDL3_DIR=${SDL3_WIN}/lib/cmake/SDL3"
fi

# ── Configure ─────────────────────────────────────────────────────────────────
echo "[build] Target:    ${TARGET}"
echo "[build] Type:      ${BUILD_TYPE}"
echo "[build] Output:    ${BUILD_DIR}"

# Allow env vars to override the compiler; CMake understands these flags too.
if [[ -n "${CC:-}" ]]; then
    echo "[build] overriding C compiler: ${CC}"
    TOOLCHAIN_ARG+=" -DCMAKE_C_COMPILER=${CC}"
fi
if [[ -n "${CXX:-}" ]]; then
    echo "[build] overriding C++ compiler: ${CXX}"
    TOOLCHAIN_ARG+=" -DCMAKE_CXX_COMPILER=${CXX}"
fi

mkdir -p "${BUILD_DIR}"

# shellcheck disable=SC2086
# TOOLCHAIN_ARG may contain multiple space-separated cmake flags; intentional word-splitting here.
cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    ${TOOLCHAIN_ARG} \
    -Wno-dev

# ── Compile ───────────────────────────────────────────────────────────────────
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

# ── Copy runtime dependencies into build dir ──────────────────────────────────
copy_deps() {
    local outdir="$1"

    if [[ "${PLATFORM}" == "Windows" ]]; then
        # With -static-libgcc/-static-libstdc++ we link everything statically,
        # so no MinGW DLLs are needed.  SDL3 is also static.
        # If you switch to shared SDL3, copy SDL3.dll here:
        # local sdl_dll="${SDL3_WIN}/bin/SDL3.dll"
        # [[ -f "${sdl_dll}" ]] && cp -v "${sdl_dll}" "${outdir}/"
        echo "[build] Windows static build – no DLLs to copy."
    else
        # Linux static SDL3 – nothing to copy.
        echo "[build] Linux static build – no shared libs to copy."
    fi
}

copy_deps "${BUILD_DIR}"

# ── Report ─────────────────────────────────────────────────────────────────────
if [[ "${PLATFORM}" == "Windows" ]]; then
    BIN="${BUILD_DIR}/bootloader.exe"
else
    BIN="${BUILD_DIR}/bootloader"
fi

echo ""
if [[ -f "${BIN}" ]]; then
    echo "[build] SUCCESS  →  ${BIN}"
else
    echo "[build] WARNING: expected binary not found at ${BIN}"
    echo "[build] Check build output above for errors."
    exit 1
fi
