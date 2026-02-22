#!/usr/bin/env bash
# scripts/run.sh
# Build (if needed) then run the bootloader.
# All arguments are forwarded to the bootloader executable.
#
# Usage:
#   bash scripts/run.sh              # terminal/headless mode (default)
#   bash scripts/run.sh --gui        # SDL3 GUI window mode
#   bash scripts/run.sh --gui --verbose

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET="${BUILD_TARGET:-linux-debug}"
BINARY="$REPO_ROOT/build/$TARGET/bootloader"

# Build the project automatically if the binary is missing
if [[ ! -f "$BINARY" ]]; then
    echo "[run.sh] Binary not found – building ($TARGET) first..."
    bash "$SCRIPT_DIR/build.sh" "$TARGET"
fi

exec "$BINARY" "$@"
