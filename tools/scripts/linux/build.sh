#!/bin/bash

# Check if the "build" directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

echo "******************************"
echo "Entering build folder"
cd build

echo "******************************"
echo "Generating build files and Building project"
cmake .. && cmake --build .

echo "******************************"
echo "Build and copy process completed."