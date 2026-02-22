#!/usr/bin/env bash
# scripts/test.sh
# Build the application and all unit tests, then run them with ctest.
# Exits non-zero if any test fails.
#
# Usage:
#   bash scripts/test.sh [BUILD_TARGET]
#
# Examples:
#   bash scripts/test.sh                 # uses linux-debug
#   bash scripts/test.sh linux-release   # uses linux-release

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET="${1:-${BUILD_TARGET:-linux-debug}}"
BUILD_DIR="$REPO_ROOT/build/$TARGET"

echo "[test.sh] Building project and tests (target: $TARGET)..."
bash "$SCRIPT_DIR/build.sh" "$TARGET"

echo "[test.sh] Running tests..."
cd "$BUILD_DIR"
ctest --output-on-failure

echo "[test.sh] All tests passed."
