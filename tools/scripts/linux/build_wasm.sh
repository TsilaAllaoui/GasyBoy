#!/bin/bash

# Check if the directory "build_wasm" exists
if [ ! -d "build_wasm" ]; then
    mkdir build_wasm
fi

cd build_wasm
emcmake cmake ..
cmake --build .
