#!/bin/bash

# Set default build type
BUILD_TYPE="Release"

# Check if user provided an argument (Debug or Release)
if [ $# -gt 0 ]; then
    if [[ "$1" == "Debug" || "$1" == "Release" ]]; then
        BUILD_TYPE=$1
    else
        echo "Invalid argument: Use 'Debug' or 'Release'."
        exit 1
    fi
fi

echo "******************************"
echo "Selected Build Type: $BUILD_TYPE"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

echo "******************************"
echo "Entering build folder"
cd build || exit 1

echo "******************************"
echo "Generating CMake files..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

echo "******************************"
echo "Building project..."
cmake --build .

echo "******************************"
echo "Build process completed."
