@echo off
echo ========================================
echo Building JitsiMeetQt Unit Tests
echo ========================================

REM Set Qt environment
set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;%PATH%

REM Create build directory
if not exist "build_unit_tests" mkdir build_unit_tests
cd build_unit_tests

echo.
echo Generating Makefile...
qmake ..\test_unit_all.pro -spec win32-g++ CONFIG+=debug

if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake failed!
    pause
    exit /b 1
)

echo.
echo Building unit tests...
mingw32-make clean
mingw32-make -j4

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Unit Tests Build Completed Successfully!
echo ========================================
echo.
echo Executable: build_unit_tests\debug\test_unit_all.exe
echo.
echo To run the tests, execute:
echo   cd build_unit_tests\debug
echo   test_unit_all.exe
echo.

cd ..
pause