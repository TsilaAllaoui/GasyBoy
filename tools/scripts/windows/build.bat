@echo off

:: Check if the "build_wasm" directory exists
if not exist "build" (
    mkdir build
)

cd build
cmake --build .
