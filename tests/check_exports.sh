#!/usr/bin/env bash
# Assert that libhello.so's dynamic symbol table contains exactly the
# documented public API and nothing else. Catches accidental leakage of
# private helpers that forgot HELLO_API + static.
#
# Argument: path to libhello.so (the unversioned symlink works fine).
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <path-to-libhello.so>" >&2
    exit 2
fi
lib="$1"

if ! command -v nm >/dev/null 2>&1; then
    echo "check_exports: nm not found in PATH; skipping" >&2
    exit 0
fi

expected=$(printf '%s\n' \
    hello_format \
    hello_greet \
    hello_status_string \
    hello_version \
    | sort)

# -D = dynamic symbol table, --defined-only excludes UND entries.
# --format=posix yields "name letter value size" with stable column 1.
actual=$(nm -D --defined-only --format=posix "$lib" \
    | awk '{print $1}' \
    | sed 's/@.*//' \
    | sort -u)

diff_out=$(diff <(echo "$expected") <(echo "$actual") || true)
if [ -n "$diff_out" ]; then
    echo "FAIL: libhello.so exports an unexpected dynamic symbol set." >&2
    echo "      Lines starting with '<' are expected-but-missing," >&2
    echo "      '>' are present-but-unexpected." >&2
    echo "$diff_out" >&2
    exit 1
fi

echo "ok: dynamic symbol table = { $(echo "$actual" | paste -sd, -) }"
