@echo off
setlocal

:: Default build type
set BUILD_TYPE=Release

:: Check if user provided an argument (Debug or Release)
if "%~1"=="" (
    echo No build type specified. Defaulting to Release.
) else (
    if /I "%~1"=="Debug" (
        set BUILD_TYPE=Debug
    ) else if /I "%~1"=="Release" (
        set BUILD_TYPE=Release
    ) else (
        echo Invalid argument: Use "Debug" or "Release".
        exit /b 1
    )
)

echo ******************************
echo Selected Build Type: %BUILD_TYPE%

:: Check if the "build" directory exists, if not, create it
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

echo ******************************
echo Entering build folder...
cd build || exit /b 1

echo ******************************
echo Generating CMake files...
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

echo ******************************
echo Building project...
cmake --build .

echo ******************************
echo Build process completed.

endlocal
