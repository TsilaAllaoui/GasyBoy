@echo off
setlocal

:: Default to Release build
set BUILD_TYPE=Release

:: Check if user provided Debug or Release as an argument
if not "%~1"=="" (
    if /I "%~1"=="Debug" set BUILD_TYPE=Debug
    if /I "%~1"=="Release" set BUILD_TYPE=Release
)

echo ******************************
echo Selected Build Type: %BUILD_TYPE%

:: Define build directory based on build type
set BUILD_DIR=build_%BUILD_TYPE%

:: Create build directory if it does not exist
if not exist "%BUILD_DIR%" (
    echo Creating build directory: %BUILD_DIR%
    mkdir "%BUILD_DIR%"
)

echo ******************************
echo Entering build folder...
cd "%BUILD_DIR%" || exit /b 1

echo ******************************
echo Generating CMake files...

:: Check if MSVC is being used (Multi-Config Generator)
cmake --version | findstr /C:"Visual Studio" >nul
if %ERRORLEVEL% == 0 (
    cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
) else (
    cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
)

echo ******************************
echo Building project...

:: Check if MSVC (Multi-Config)
if %ERRORLEVEL% == 0 (
    cmake --build . --config %BUILD_TYPE%
) else (
    cmake --build .
)

echo ******************************
echo Build process completed in %BUILD_DIR%.

endlocal
