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
emcmake cmake .. && cmake --build .

echo "******************************"
echo "Copying WASM file to docs directory"
# Ensure docs directory exists
if [ ! -d "../docs" ]; then
    echo "Creating docs directory..."
    mkdir ../docs
fi

# Copy WASM file
echo "Copying WASM file..."
cp -f *.wasm ../docs/

echo "******************************"
echo "Build and copy process completed."
