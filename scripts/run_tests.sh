#!/usr/bin/env sh
# Minimal test runner; assumes project built in ./build

set -eu

BUILD_DIR="$(dirname "$0")/../build"

if [ ! -d "$BUILD_DIR" ]; then
  echo "build/ not found. Build the project into ./build first." >&2
  exit 1
fi

cd "$BUILD_DIR"
ctest --output-on-failure