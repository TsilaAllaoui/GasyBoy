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
set BUILD_DIR=build

echo ******************************
echo Entering build folder...
cd "%BUILD_DIR%" || exit /b 1

echo ******************************
echo Checking for compilers...


echo Visual Studio detected! Using MSVC generator.
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%


echo ******************************
echo Building project...

:: Use Ninja if detected for Clang or GCC
where ninja >nul 2>&1
if %ERRORLEVEL% == 0 (
    ninja
) else (
    :: Use Makefiles or MSVC build command
    where gcc >nul 2>&1
    if %ERRORLEVEL% == 0 (
        mingw32-make
    ) else (
        cmake --build . --config %BUILD_TYPE%
    )
)

echo ******************************
echo Build process completed in %BUILD_DIR%.

endlocal
