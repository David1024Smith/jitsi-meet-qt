@echo off
echo Building Performance Optimization Test...

REM Set Qt environment
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM Create build directory
if not exist "build_performance_test" mkdir build_performance_test
cd build_performance_test

REM Generate Makefile
qmake -o Makefile ..\test_performance_optimization.pro

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

echo Performance optimization test built successfully!
echo Executable: build_performance_test\test_performance_optimization.exe

cd ..