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

# colour helpers
GREEN="\e[32m"
YELLOW="\e[33m"
RED="\e[31m"
RESET="\e[0m"
info() { echo -e "${GREEN}[run]${RESET} $*"; }
warn() { echo -e "${YELLOW}[run]${RESET} $*"; }
error() { echo -e "${RED}[run]${RESET} $*"; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET="${BUILD_TARGET:-linux-debug}"
BINARY="$REPO_ROOT/build/$TARGET/bootloader"

# Build the project automatically if the binary is missing
if [[ ! -f "$BINARY" ]]; then
    info "Binary not found – building ($TARGET) first..."
    bash "$SCRIPT_DIR/build.sh" "$TARGET"
fi

# if no args provided default to GUI mode (was previously headless by default)
if [[ $# -eq 0 ]]; then
    set -- "--gui"
fi

# change into the build directory so logs/seq folders are created there
BUILD_DIR="$REPO_ROOT/build/$TARGET"
cd "$BUILD_DIR"
# ensure bin directories exist
mkdir -p bin/logs
mkdir -p bin/seq


# monitoring support: if --monitor passed, run in background and tail logs
info "Working directory: $BUILD_DIR"
if [[ "$1" == "--monitor" ]]; then
    shift
    ./bootloader "$@" &
    pid=$!
    # tail all existing/future log files
    tail -F bin/logs/*.log &
    tailpid=$!
    wait $pid
    kill $tailpid 2>/dev/null || true
    exit 0
fi

exec "./bootloader" "$@"
