#!/usr/bin/env bash
# run.sh – Build (if needed) and launch the WASM Quine Bootloader.
# Run from the repository root, or with: bash scripts/run.sh

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BINARY="${REPO_ROOT}/build/wasm_quine_bootloader"

# Build first if the binary does not exist
if [[ ! -f "${BINARY}" ]]; then
    echo "[run] Binary not found, building first..."
    bash "${REPO_ROOT}/scripts/build.sh"
fi

echo "[run] Starting WASM Quine Bootloader..."
exec "${BINARY}" "$@"
