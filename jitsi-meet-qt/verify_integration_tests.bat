@echo off
echo Verifying Integration Test Compilation...
echo.

cd /d "%~dp0"

REM Check if build directory exists
if not exist "build" (
    echo Creating build directory...
    mkdir build
    cd build
    cmake ..
    cd ..
)

REM Try to build the integration test
echo Building integration test...
cd build
cmake --build . --target test_integration

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✓ Integration test compiled successfully!
    echo.
    echo Test executable location: build\tests\test_integration.exe
    echo.
    echo To run the tests, use:
    echo   run_integration_tests.bat
    echo.
) else (
    echo.
    echo ✗ Integration test compilation failed!
    echo Please check the build output above for errors.
    echo.
)

cd ..
pause