#!/bin/bash

# Check if the directory "build_wasm" exists
if [ ! -d "build_wasm" ]; then
    mkdir build_wasm
fi

cd build_wasm
emcmake cmake ..
cmake --build .

# Ensure docs directory exists
mkdir -p ../docs

# Copy WASM to docs
cp *.wasm ../docs/ 2>/dev/null || echo "No WASM-related files found."
