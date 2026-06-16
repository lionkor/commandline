#!/bin/bash
# Regression test for #42: GNU screen -dmS causes commandline to break and CPU spin.
#
# Simulates the detached-screen scenario: InteractiveBackend is selected
# (forced via env var) but stdin is /dev/null (immediate EOF, as happens
# when screen detaches from the TTY).  The input thread must detect EOF
# and shut down instead of spinning.
set -euo pipefail

BUILD_DIR="${1:-build}"
BINARY="$BUILD_DIR/commandline_test"

# Build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON >/dev/null 2>&1
make -j"$(nproc)" commandline_test >/dev/null 2>&1
cd - >/dev/null

# Run with forced interactive + EOF on stdin for 3 seconds.
# /usr/bin/time -v captures CPU time so we can detect a spin.
TIMEOUT=3
TIME_OUT=$(mktemp)

set +e
COMMANDLINE_FORCE_INTERACTIVE=1 \
    /usr/bin/time -o "$TIME_OUT" -v timeout "$TIMEOUT" "$BINARY" \
        < /dev/null >/dev/null 2>/dev/null
TIMEOUT_RC=$?
set -e

# Parse user and system CPU time from time output.
user_time=$(awk '/User time \(seconds\)/ {print $NF}' "$TIME_OUT")
sys_time=$(awk '/System time \(seconds\)/ {print $NF}' "$TIME_OUT")
rm -f "$TIME_OUT"

if [ -z "$user_time" ] || [ -z "$sys_time" ]; then
    echo "FAIL: could not parse CPU times from /usr/bin/time output"
    exit 1
fi

total_cpu=$(awk "BEGIN {print $user_time + $sys_time}")

# If the process used more than 0.3 s of CPU in $TIMEOUT seconds,
# it was spinning.
threshold=0.3
if awk "BEGIN {exit !($total_cpu > $threshold)}"; then
    echo "FAIL: CPU spin detected: ${total_cpu}s CPU in ${TIMEOUT}s wall time"
    echo "User=${user_time}s  Sys=${sys_time}s  Threshold=${threshold}s"
    exit 1
fi

echo "PASS: no CPU spin (${total_cpu}s CPU in ${TIMEOUT}s)"
