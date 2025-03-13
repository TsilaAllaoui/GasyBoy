#!/bin/bash

# Check if the "build_wasm" directory exists
if not exist "build_wasm" (
    echo "Creating build_wasm directory..."
    mkdir build_wasm
)

echo "******************************"
echo "Entering build folder"
cd build_wasm

echo "******************************"
echo "Generating build files and Building project"
emcmake cmake .. && cmake --build .

echo "******************************"
echo "Copying WASM file to docs directory"
# Ensure docs directory exists
if not exist "..\docs" (
    echo "Creating docs directory..."
    mkdir ..\docs
)

# Copy WASM file
echo "Copying WASM file..."
copy /Y *.wasm ..\docs\

echo "******************************"
echo "Build and copy process completed."
