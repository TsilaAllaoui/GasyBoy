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
echo Checking for GCC...

where gcc >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo GCC found! Using MinGW Makefiles.
    cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else (
    echo GCC not found, checking for MSVC...

    cmake --version | findstr /C:"Visual Studio" >nul
    if %ERRORLEVEL% == 0 (
        echo Visual Studio detected! Using MSVC generator.
        cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
    ) else (
        echo No GCC or MSVC detected! Exiting.
        exit /b 1
    )
)

echo ******************************
echo Building project...

where gcc >nul 2>&1
if %ERRORLEVEL% == 0 (
    mingw32-make
) else (
    cmake --build . --config %BUILD_TYPE%
)

echo ******************************
echo Build process completed in %BUILD_DIR%.

endlocal
