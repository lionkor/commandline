#!/usr/bin/env bash
# Minimal clang-format check script for CI/CD and local testing

set -euo pipefail

# Ensure clang-format is available
if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format not found on PATH."
  echo "Install it (e.g., apt-get install -y clang-format, brew install clang-format) and retry."
  exit 127
fi

clang-format --version

# Collect tracked C/C++ files
FILES=$(git ls-files '*.[ch]' '*.cc' '*.cpp' '*.cxx' '*.hh' '*.hpp' '*.hxx' || true)

# Exclusion list (extensible)
EXCLUDE=("tests/doctest.h")

# Filter out excluded files
if [ -n "${FILES}" ]; then
  FILTERED_FILES=$(printf "%s\n" ${FILES} | grep -v -F -x $(printf -- " -e %s" "${EXCLUDE[@]}"))
else
  FILTERED_FILES=""
fi

if [ -z "${FILTERED_FILES}" ]; then
  echo "No C/C++ source files to check."
  exit 0
fi

# Check formatting according to .clang-format (.style=file)
# -n: dry-run, don't modify files
# --Werror: treat formatting changes as errors
clang-format -style=file -n --Werror ${FILTERED_FILES}
