#!/usr/bin/env bash
# Minimal cross-platform CMake configure + build script for CI/local use

set -euo pipefail

# Configurable via environment variables:
# - BUILD_TYPE: CMake build type (Debug, Release, RelWithDebInfo, MinSizeRel). Default: Release
# - COMMANDLINE_LIBTYPE: Project-specific library type (e.g., STATIC|SHARED). Default: SHARED
# - CMAKE_GENERATOR: Optional CMake generator (e.g., Ninja, "Unix Makefiles", "Visual Studio 17 2022")
# - BUILD_DIR: Build directory. Default: <repo_root>/build

BUILD_TYPE="${BUILD_TYPE:-Release}"
LIBTYPE="${COMMANDLINE_LIBTYPE:-SHARED}"

# Resolve repo root as parent of this script dir
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"

# Ensure cmake exists
if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake not found on PATH. Install cmake and retry."
  exit 127
fi

# Optional generator flag
GEN_ARGS=()
if [ -n "${CMAKE_GENERATOR:-}" ]; then
  GEN_ARGS+=("-G" "${CMAKE_GENERATOR}")
fi

# Configure
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCOMMANDLINE_LIBTYPE="${LIBTYPE}" \
  "${GEN_ARGS[@]}"

# Build
cmake --build "${BUILD_DIR}" --parallel --config "${BUILD_TYPE}"
