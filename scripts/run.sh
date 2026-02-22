#!/usr/bin/env bash
# run.sh – Build (if needed) and launch the WASM Quine Bootloader.
#
# Usage:  bash scripts/run.sh [TARGET]
#
# TARGET defaults to linux-debug (same as build.sh).

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="${1:-linux-debug}"

case "${TARGET}" in
    windows-debug|windows-release)
        BIN="${REPO_ROOT}/build/${TARGET}/bootloader.exe"
        ;;
    *)
        BIN="${REPO_ROOT}/build/${TARGET}/bootloader"
        ;;
esac

if [[ ! -f "${BIN}" ]]; then
    echo "[run] Binary not found for target '${TARGET}', building first..."
    bash "${REPO_ROOT}/scripts/build.sh" "${TARGET}"
fi

echo "[run] Starting WASM Quine Bootloader (${TARGET})..."
exec "${BIN}" "$@"
