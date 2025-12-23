#!/usr/bin/env bash
set -euo pipefail

# Requirements (Ubuntu/Debian):
#   sudo apt-get update
#   sudo apt-get install -y cmake ninja-build mingw-w64

mkdir -p build
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
echo "Built: build/MetricsAgent.exe"
