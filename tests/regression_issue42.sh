#!/bin/bash
# Regression test for #42: GNU screen -dmS causes commandline to break and CPU spin.
#
# Forced interactive mode + /dev/null stdin simulates the detached-screen scenario:
# stdin reaches EOF immediately.  The input thread must detect EOF and shut down
# instead of spinning.
set -euo pipefail

BINARY="${1:-build}/commandline_test"

TIME_OUT=$(mktemp)
trap 'rm -f "$TIME_OUT"' EXIT

COMMANDLINE_FORCE_INTERACTIVE=1 \
    /usr/bin/time -o "$TIME_OUT" -v timeout 3 "$BINARY" \
        < /dev/null >/dev/null 2>/dev/null || true

total_cpu=$(awk '
    /User time \(seconds\)/  { u = $NF }
    /System time \(seconds\)/ { s = $NF }
    END {
        if (u == "" || s == "") exit 1
        print u + s
    }
' "$TIME_OUT") || { echo "FAIL: could not parse CPU time from /usr/bin/time output"; exit 1; }

if awk "BEGIN {exit !($total_cpu >= 0.3)}"; then
    echo "FAIL: CPU spin detected: ${total_cpu}s CPU in 3s"
    exit 1
fi

echo "PASS: no CPU spin (${total_cpu}s CPU in 3s)"
