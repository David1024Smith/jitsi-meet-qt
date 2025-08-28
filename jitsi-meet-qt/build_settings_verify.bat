@echo off
echo Building SettingsDialog verification...

REM Set Qt environment (adjust path as needed)
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%

REM Create build directory
if not exist build_settings_verify mkdir build_settings_verify
cd build_settings_verify

REM Configure with CMake
cmake -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=%QT_DIR% ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_SETTINGS_VERIFY=ON ^
    ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed
    pause
    exit /b 1
)

REM Build the verification
mingw32-make verify_settings_dialog

if %ERRORLEVEL% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

echo Build completed successfully
echo Running verification...
verify_settings_dialog.exe

pause