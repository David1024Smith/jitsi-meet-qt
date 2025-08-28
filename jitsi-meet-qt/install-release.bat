@echo off
echo Installing and Packaging Jitsi Meet Qt
echo ======================================

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Check if build exists
if not exist build-release (
    echo Error: Release build not found. Run build-release.bat first.
    pause
    exit /b 1
)

cd build-release

REM Install the application
echo Installing application...
cmake --install . --config Release

if %ERRORLEVEL% neq 0 (
    echo Installation failed!
    pause
    exit /b 1
)

REM Create installer package
echo Creating installer package...
cpack -G NSIS

if %ERRORLEVEL% neq 0 (
    echo Package creation failed!
    pause
    exit /b 1
)

echo Installation and packaging completed successfully!
echo Installer package created in build-release directory.
pause