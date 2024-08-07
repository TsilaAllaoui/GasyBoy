#!/bin/bash

# Define the path to the executable
EXECUTABLE_PATH="../build/tests/gasyboy_tests"

# Check if the executable exists
if [ -f "$EXECUTABLE_PATH" ]; then
    echo "Running executable..."
    "$EXECUTABLE_PATH"
    exit 0
else
    echo "Error: Executable not found."
    exit 1
fi
