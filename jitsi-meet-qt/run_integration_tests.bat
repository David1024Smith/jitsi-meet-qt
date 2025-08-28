@echo off
echo Running Jitsi Meet Qt Integration Tests...
echo.

cd /d "%~dp0"

REM Build the tests if needed
if not exist "build\tests\test_integration.exe" (
    echo Building integration tests...
    cd build
    cmake --build . --target test_integration
    cd ..
)

REM Run the integration tests
echo Starting integration tests...
echo.

build\tests\test_integration.exe

echo.
echo Integration tests completed.
pause