#!/usr/bin/env bash
# Assert that every HELLO_API-tagged declaration in the public header has a
# preceding Doxygen comment block. Mirrors the policy-as-test pattern used
# by check_exports.sh: the contract is documented in writing, and the test
# fails the build if a future change drifts away from it.
#
# Argument: path to hello.h.
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <path-to-hello.h>" >&2
    exit 2
fi
hdr="$1"

# Track Doxygen comment blocks explicitly. A HELLO_API line is considered
# documented iff a /** ... */ block (single-line or multi-line) closed
# *immediately* before it, with only blank lines in between. Plain `/* */`
# comments and `#endif /* X */` markers do not count.
undoc=$(awk '
    BEGIN { in_doc = 0; doc_seen = 0 }
    # Single-line /** brief */
    /^[[:space:]]*\/\*\*.*\*\/[[:space:]]*$/ { doc_seen = 1; next }
    # Opening of multi-line /** ...
    /^[[:space:]]*\/\*\*/ { in_doc = 1; next }
    # Closing of multi-line ... */
    in_doc && /\*\// { in_doc = 0; doc_seen = 1; next }
    in_doc { next }
    /^[[:space:]]*$/ { next }
    /^HELLO_API/ {
        if (!doc_seen) { print FILENAME ":" NR ": " $0 }
        doc_seen = 0
        next
    }
    { doc_seen = 0 }
' "$hdr")

if [ -n "$undoc" ]; then
    echo "FAIL: HELLO_API declaration without a preceding /** ... */ block:" >&2
    echo "$undoc" >&2
    exit 1
fi

count=$(grep -c '^HELLO_API' "$hdr" || true)
echo "ok: ${count} HELLO_API declaration(s) all carry a Doxygen comment"
