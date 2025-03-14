@echo off

:: Check if the "build" directory exists
if not exist "build" (
    echo "Creating build directory..."
    mkdir build_wasm
)

echo "******************************"
echo "Entering build folder"
cd build

echo "******************************"
echo "Generating build files and Building project"
cmake .. && cmake --build .

echo "******************************"
echo "Build and copy process completed."
