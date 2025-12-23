#!/usr/bin/env bash
set -euo pipefail
mkdir -p build
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw-w64/toolchain-x86_64.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
echo "Built: build/MetricsAgent.exe"
