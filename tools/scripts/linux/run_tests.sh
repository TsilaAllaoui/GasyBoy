#!/bin/bash

# Define paths to the executable for both Debug and Release configurations
DEBUG_EXECUTABLE_PATH="../../build/Debug/gasyboy_cpu_tester.exe"
RELEASE_EXECUTABLE_PATH="../../build/Release/gasyboy_cpu_tester.exe"

# Check if the Debug executable exists
if [ -f "$DEBUG_EXECUTABLE_PATH" ]; then
    echo "Running Debug executable..."
    "$DEBUG_EXECUTABLE_PATH"
    exit 0
fi

# Check if the Release executable exists
if [ -f "$RELEASE_EXECUTABLE_PATH" ]; then
    echo "Running Release executable..."
    "$RELEASE_EXECUTABLE_PATH"
    exit 0
fi

# If neither executable was found
echo "Error: Executable not found in Debug or Release directories."
exit 1
