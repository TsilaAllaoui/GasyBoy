#!/bin/bash

# Check if the "build_wasm" directory exists
if [ ! -d "build_wasm" ]; then
    echo "Creating build_wasm directory..."
    mkdir build_wasm
fi

echo "******************************"
echo "Entering build folder"
cd build_wasm

echo "******************************"
echo "Generating build files and Building project"
cmake .. && cmake --build .

echo "******************************"
echo "Build and copy process completed."