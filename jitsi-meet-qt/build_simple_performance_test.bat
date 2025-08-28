@echo off
echo Building Simple Performance Test...

REM Set Qt environment
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM Create build directory
if not exist "build_simple_performance" mkdir build_simple_performance
cd build_simple_performance

REM Generate Makefile
qmake -o Makefile ..\test_performance_simple.pro

REM Check if qmake succeeded
if %ERRORLEVEL% neq 0 (
    echo Error: qmake failed
    cd ..
    exit /b 1
)

REM Build the test
mingw32-make

REM Check if build succeeded
if %ERRORLEVEL% neq 0 (
    echo Error: Build failed
    cd ..
    exit /b 1
)

echo Simple performance test built successfully!
echo Running test...

REM Run the test
test_performance_simple.exe

cd ..