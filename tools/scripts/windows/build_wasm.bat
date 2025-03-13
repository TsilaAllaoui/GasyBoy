@echo off

:: Check if the "build_wasm" directory exists
if not exist "build_wasm" (
    mkdir build_wasm
)

cd build_wasm
emcmake cmake ..
cmake --build .

:: Ensure docs directory exists
if not exist "../docs" mkdir ../docs

:: Copy WASM file
copy /Y *.wasm ..\docs\