@echo off

setlocal

REM Define the paths
set DEBUG_PATH=..\build\tests\Debug\gasyboy_tests.exe
set RELEASE_PATH=..\build\tests\Release\gasyboy_tests.exe

REM Check if Debug executable exists
if exist "%DEBUG_PATH%" (
    echo Running Debug executable...
    "%DEBUG_PATH%"
    goto :EOF
)

REM Check if Release executable exists
if exist "%RELEASE_PATH%" (
    echo Running Release executable...
    "%RELEASE_PATH%"
    goto :EOF
)

REM If neither executable exists, print an error and exit with failure
echo Error: Neither Debug nor Release executable found.
exit /b 1
