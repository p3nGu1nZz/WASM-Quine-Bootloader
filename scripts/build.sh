#!/usr/bin/env bash
# build.sh – Configure and build the WASM Quine Bootloader.
# Run from the repository root, or with: bash scripts/build.sh
#
# Options (environment variables):
#   BUILD_TYPE   – Debug | Release (default: Release)
#   JOBS         – parallel jobs    (default: nproc)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc)}"
BUILD_DIR="${REPO_ROOT}/build"

echo "[build] Source:  ${REPO_ROOT}"
echo "[build] Type:    ${BUILD_TYPE}"
echo "[build] Jobs:    ${JOBS}"
echo "[build] Output:  ${BUILD_DIR}"

cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -Wno-dev

cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

echo ""
echo "[build] Success!  Binary: ${BUILD_DIR}/wasm_quine_bootloader"
