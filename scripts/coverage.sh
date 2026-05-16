#!/usr/bin/env bash
# Run coverage analysis locally with flags appropriate to the installed lcov.
# The CI workflow uses lcov 2.x's stricter flag surface; older distros
# (Debian 12 ships lcov 1.16) reject those flags. This script auto-detects
# the version and picks a compatible set.
#
# Usage: scripts/coverage.sh [BUILD_DIR]   (default: build/coverage)
set -euo pipefail

cd "$(dirname "$0")/.."

BUILD_DIR="${1:-build/coverage}"
FLOOR="${MIN_LINE_COVERAGE:-90}"

if ! command -v lcov >/dev/null 2>&1; then
    echo "error: lcov not found in PATH" >&2
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "error: $BUILD_DIR does not exist." >&2
    echo "       Run: cmake --preset coverage && cmake --build --preset coverage && ctest --test-dir $BUILD_DIR" >&2
    exit 1
fi

# Major version detection. lcov 2.x prints "lcov: LCOV version 2.x"; 1.x
# prints "lcov: LCOV version 1.x".
lcov_major=$(lcov --version 2>&1 | sed -nE 's/.*LCOV version ([0-9]+).*/\1/p' | head -1)

capture_args=(--capture --directory "$BUILD_DIR" --output-file "$BUILD_DIR/coverage.info")
remove_args=(--remove "$BUILD_DIR/coverage.info" '/usr/*' '*/tests/*' --output-file "$BUILD_DIR/coverage.info")

if [ "${lcov_major:-1}" -ge 2 ]; then
    # 2.x: strict about macro-expanded line mismatches and unused remove
    # globs; renamed --rc lcov_branch_coverage -> branch_coverage.
    capture_args+=(--ignore-errors mismatch --rc branch_coverage=1)
    remove_args+=(--ignore-errors unused)
else
    # 1.x: original flag names; tolerant by default.
    capture_args+=(--rc lcov_branch_coverage=1)
fi

lcov "${capture_args[@]}"
lcov "${remove_args[@]}"
lcov --list "$BUILD_DIR/coverage.info"

# Threshold check, matching .github/workflows/ci.yml's logic.
pct=$(lcov --summary "$BUILD_DIR/coverage.info" 2>&1 \
       | awk '/^ *lines\.+:/ {gsub("%",""); print $2; exit}')
if [ -z "$pct" ]; then
    echo "error: could not parse line coverage from lcov --summary" >&2
    exit 1
fi
echo "line coverage: ${pct}% (floor: ${FLOOR}%)"
if ! awk -v p="$pct" -v f="$FLOOR" 'BEGIN { exit (p+0 >= f+0) ? 0 : 1 }'; then
    echo "error: line coverage ${pct}% < ${FLOOR}%" >&2
    exit 1
fi
