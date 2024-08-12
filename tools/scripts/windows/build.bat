@echo off

REM Define paths
SET BUILD_DIR=..\..\..\build
SEt ORIGIN_DIR=..\tools\scripts\windows

REM Create the build directory if it doesn't exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Change to the build directory
cd "%BUILD_DIR%"

REM Run CMake to configure the project from the parent directory
cmake ..

REM Build the project
cmake --build .

@REM REM Optionally, you can run your custom build.bat script if needed
@REM REM call "..\..\tools\scripts\windows\build.bat"

cd "%ORIGIN_DIR%"

echo Build process completed.
