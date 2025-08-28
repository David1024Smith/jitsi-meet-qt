@echo off
echo Building Jitsi Meet Qt Theme Test...

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_theme_test mkdir build_theme_test
cd build_theme_test

REM Configure with CMake
cmake -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=%QT_DIR% ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_THEME_TEST=ON ^
    ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
mingw32-make -j4

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Running theme test...

REM Run the theme test
test_theme_resources.exe

pause