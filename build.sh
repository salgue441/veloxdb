#!/bin/bash
set -e
echo "Building VeloxDB"

# Build C++ First
echo "Building C++ Core"

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# Build Rust Components
echo "Building Rust components"
export VELOX_CORE_LIB_DIR="$(pwd)/build"
cargo build --release

echo "Build completed successfully"