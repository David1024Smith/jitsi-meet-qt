@echo off
echo Testing CMake Configuration
echo ===========================

REM Create a test build directory
if exist test-build rmdir /s /q test-build
mkdir test-build
cd test-build

REM Test CMake configuration
echo Testing CMake configuration...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration test failed!
    cd ..
    pause
    exit /b 1
)

echo CMake configuration test passed!
cd ..

REM Clean up
rmdir /s /q test-build

echo Test completed successfully!
pause