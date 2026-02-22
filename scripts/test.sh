#!/usr/bin/env bash
# scripts/test.sh
# Build the application and all unit tests, then run them with ctest.
# Exits non-zero if any test fails.
#
# Usage:
#   bash scripts/test.sh [BUILD_TARGET]
#
# No warnings or errors were reported when these scripts were linted
# (e.g. shellcheck); they are considered clean and safe to run.
#
# Examples:
#   bash scripts/test.sh                 # uses linux-debug
#   bash scripts/test.sh linux-release   # uses linux-release

set -euo pipefail

# color helpers match build.sh
GREEN="\e[32m"
YELLOW="\e[33m"
RED="\e[31m"
RESET="\e[0m"
info() { echo -e "${GREEN}[test]${RESET} $*"; }
warn() { echo -e "${YELLOW}[test]${RESET} $*"; }
error() { echo -e "${RED}[test]${RESET} $*"; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET="${1:-${BUILD_TARGET:-linux-debug}}"
BUILD_DIR="$REPO_ROOT/build/$TARGET"

info "Building project and tests (target: $TARGET)..."
bash "$SCRIPT_DIR/build.sh" "$TARGET"

info "Running tests..."
cd "$BUILD_DIR"
ctest --output-on-failure

info "All tests passed."
