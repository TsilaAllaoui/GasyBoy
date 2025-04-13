@echo off
setlocal

:: Default to Release if not specified
set BUILD_TYPE=Release
if /I "%~1"=="Debug" set BUILD_TYPE=Debug
if /I "%~1"=="Release" set BUILD_TYPE=Release

echo ==============================
echo Selected Build Type: %BUILD_TYPE%
echo ==============================

:: Create build folder if it doesn't exist
set BUILD_DIR=build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%" || exit /b 1

:: Generating project 
echo ==============================
echo Generating project files...
echo ==============================

cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

:: Building project
echo ==============================
echo Building project...
echo ==============================

cmake --build . --config %BUILD_TYPE%

echo ==============================
echo Build completed in: %BUILD_DIR%
endlocal
