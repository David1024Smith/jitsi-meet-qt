@echo off
echo Building SettingsDialog test...

REM Set Qt environment
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM Create build directory
if not exist build_settings_test mkdir build_settings_test
cd build_settings_test

REM Configure with CMake
cmake -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=%QT_DIR% ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_SETTINGS_TEST=ON ^
    ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed
    pause
    exit /b 1
)

REM Build the test
mingw32-make test_settings_dialog

if %ERRORLEVEL% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

echo Build completed successfully
echo Running test...
test_settings_dialog.exe

pause