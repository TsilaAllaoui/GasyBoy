@echo off

:: Check if the "build_wasm" directory exists
if not exist "build_wasm" (
    echo Creating build_wasm directory...
    mkdir build_wasm
)

:: Entering build folder
echo =====================
echo Entering build folder
echo =====================
cd build_wasm

:: Building project
echo  ==========================================
echo Generating build files and Building project
echo  ==========================================
emcmake cmake .. && cmake --build .

echo ===================================
echo Copying WASM file to docs directory
echo ===================================
:: Ensure docs directory exists
if not exist "../docs" (
    echo Creating docs directory...
    mkdir ../docs
)

:: Copy WASM file
echo Copying WASM file...
copy /Y *.wasm ..\docs\

echo =================================
echo Build and copy process completed
echo =================================