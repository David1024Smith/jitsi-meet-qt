@echo off
REM Jitsi Meet Qt - 依赖项安装脚本
REM 自动检测和安装构建所需的依赖项

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Dependency Installer
echo ========================================

REM 依赖项配置
set QT_VERSION=5.15.2
set MINGW_VERSION=810
set CMAKE_MIN_VERSION=3.16
set NSIS_VERSION=3.08

echo Checking system requirements...

REM 检查操作系统
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
echo Windows Version: %VERSION%

if "%VERSION%"=="10.0" (
    echo Windows 10: Supported
) else if "%VERSION%"=="6.3" (
    echo Windows 8.1: Supported
) else if "%VERSION%"=="6.1" (
    echo Windows 7: Supported with limitations
) else (
    echo Warning: Untested Windows version
)

REM 检查系统架构
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
    echo Architecture: x64
) else (
    set ARCH=x86
    echo Architecture: x86 ^(not recommended^)
)

echo.
echo ========================================
echo Checking Dependencies
echo ========================================

REM 检查Qt安装
echo Checking Qt %QT_VERSION%...
set QT_FOUND=0
set QT_PATHS[0]=C:\Qt\%QT_VERSION%\mingw81_64
set QT_PATHS[1]=C:\Qt\Qt%QT_VERSION%\%QT_VERSION%\mingw81_64

for /L %%i in (0,1,1) do (
    if exist "!QT_PATHS[%%i]!\bin\qmake.exe" (
        set QT_DIR=!QT_PATHS[%%i]!
        set QT_FOUND=1
        echo   Qt %QT_VERSION%: Found at !QT_DIR!
        goto :qt_check_done
    )
)

if %QT_FOUND%==0 (
    echo   Qt %QT_VERSION%: Not found
    set INSTALL_QT=1
) else (
    set INSTALL_QT=0
)
:qt_check_done

REM 检查MinGW
echo Checking MinGW...
set MINGW_FOUND=0
set MINGW_PATHS[0]=C:\Qt\Tools\mingw810_64
set MINGW_PATHS[1]=C:\mingw64

for /L %%i in (0,1,1) do (
    if exist "!MINGW_PATHS[%%i]!\bin\gcc.exe" (
        set MINGW_DIR=!MINGW_PATHS[%%i]!
        set MINGW_FOUND=1
        echo   MinGW: Found at !MINGW_DIR!
        goto :mingw_check_done
    )
)

if %MINGW_FOUND%==0 (
    echo   MinGW: Not found
    set INSTALL_MINGW=1
) else (
    set INSTALL_MINGW=0
)
:mingw_check_done

REM 检查CMake
echo Checking CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo   CMake: Not found
    set INSTALL_CMAKE=1
) else (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr "cmake version"') do set CMAKE_VER=%%i
    echo   CMake: Found version !CMAKE_VER!
    set INSTALL_CMAKE=0
)

REM 检查NSIS
echo Checking NSIS...
makensis /VERSION >nul 2>&1
if errorlevel 1 (
    echo   NSIS: Not found
    set INSTALL_NSIS=1
) else (
    for /f %%i in ('makensis /VERSION') do set NSIS_VER=%%i
    echo   NSIS: Found version !NSIS_VER!
    set INSTALL_NSIS=0
)

REM 检查Git
echo Checking Git...
git --version >nul 2>&1
if errorlevel 1 (
    echo   Git: Not found
    set INSTALL_GIT=1
) else (
    for /f "tokens=3" %%i in ('git --version') do set GIT_VER=%%i
    echo   Git: Found version !GIT_VER!
    set INSTALL_GIT=0
)

echo.
echo ========================================
echo Installation Plan
echo ========================================

set NEED_INSTALL=0

if %INSTALL_QT%==1 (
    echo - Install Qt %QT_VERSION% with MinGW
    set NEED_INSTALL=1
)

if %INSTALL_MINGW%==1 (
    if %INSTALL_QT%==0 (
        echo - Install MinGW %MINGW_VERSION%
        set NEED_INSTALL=1
    )
)

if %INSTALL_CMAKE%==1 (
    echo - Install CMake
    set NEED_INSTALL=1
)

if %INSTALL_NSIS%==1 (
    echo - Install NSIS
    set NEED_INSTALL=1
)

if %INSTALL_GIT%==1 (
    echo - Install Git
    set NEED_INSTALL=1
)

if %NEED_INSTALL%==0 (
    echo All dependencies are already installed!
    echo.
    echo You can now run: build-all.bat
    pause
    goto :eof
)

echo.
set /p PROCEED="Do you want to proceed with installation? (y/n): "
if /i not "%PROCEED%"=="y" (
    echo Installation cancelled.
    pause
    goto :eof
)

echo.
echo ========================================
echo Installing Dependencies
echo ========================================

REM 创建临时下载目录
set TEMP_DIR=%TEMP%\jitsi-qt-deps
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"

REM 安装Qt
if %INSTALL_QT%==1 (
    echo.
    echo Installing Qt %QT_VERSION%...
    echo.
    echo Qt requires manual installation. Please:
    echo 1. Visit: https://www.qt.io/download-qt-installer
    echo 2. Download Qt Online Installer
    echo 3. Install Qt %QT_VERSION% with MinGW %MINGW_VERSION% 64-bit
    echo 4. Make sure to select:
    echo    - Qt %QT_VERSION% ^> MinGW 8.1.0 64-bit
    echo    - Qt %QT_VERSION% ^> Sources
    echo    - Developer and Designer Tools ^> MinGW 8.1.0 64-bit
    echo    - Developer and Designer Tools ^> CMake
    echo.
    echo Press any key when Qt installation is complete...
    pause >nul
    
    REM 验证Qt安装
    for /L %%i in (0,1,1) do (
        if exist "!QT_PATHS[%%i]!\bin\qmake.exe" (
            set QT_DIR=!QT_PATHS[%%i]!
            set QT_FOUND=1
            echo Qt installation verified at: !QT_DIR!
            goto :qt_install_done
        )
    )
    
    echo Error: Qt installation not found after installation
    echo Please check the installation and try again
    pause
    exit /b 1
    
    :qt_install_done
)

REM 安装CMake（如果Qt安装没有包含）
if %INSTALL_CMAKE%==1 (
    cmake --version >nul 2>&1
    if errorlevel 1 (
        echo.
        echo Installing CMake...
        echo.
        echo CMake requires manual installation. Please:
        echo 1. Visit: https://cmake.org/download/
        echo 2. Download CMake Windows x64 Installer
        echo 3. Run the installer and add CMake to PATH
        echo.
        echo Press any key when CMake installation is complete...
        pause >nul
        
        REM 验证CMake安装
        cmake --version >nul 2>&1
        if errorlevel 1 (
            echo Error: CMake not found after installation
            echo Please add CMake to your PATH and try again
            pause
            exit /b 1
        )
        
        echo CMake installation verified
    )
)

REM 安装NSIS
if %INSTALL_NSIS%==1 (
    echo.
    echo Installing NSIS...
    echo.
    echo NSIS requires manual installation. Please:
    echo 1. Visit: https://nsis.sourceforge.io/Download
    echo 2. Download NSIS %NSIS_VERSION% or later
    echo 3. Run the installer and add NSIS to PATH
    echo.
    echo Press any key when NSIS installation is complete...
    pause >nul
    
    REM 验证NSIS安装
    makensis /VERSION >nul 2>&1
    if errorlevel 1 (
        echo Error: NSIS not found after installation
        echo Please add NSIS to your PATH and try again
        pause
        exit /b 1
    )
    
    echo NSIS installation verified
)

REM 安装Git
if %INSTALL_GIT%==1 (
    echo.
    echo Installing Git...
    echo.
    echo Git requires manual installation. Please:
    echo 1. Visit: https://git-scm.com/download/win
    echo 2. Download Git for Windows
    echo 3. Run the installer with default settings
    echo.
    echo Press any key when Git installation is complete...
    pause >nul
    
    REM 验证Git安装
    git --version >nul 2>&1
    if errorlevel 1 (
        echo Error: Git not found after installation
        echo Please add Git to your PATH and try again
        pause
        exit /b 1
    )
    
    echo Git installation verified
)

REM 清理临时文件
if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%"

echo.
echo ========================================
echo Installation Complete!
echo ========================================

echo All dependencies have been installed successfully.
echo.
echo Installed components:
if %QT_FOUND%==1 echo - Qt %QT_VERSION% at %QT_DIR%
if %MINGW_FOUND%==1 echo - MinGW at %MINGW_DIR%
cmake --version | findstr "cmake version"
makensis /VERSION 2>nul && echo - NSIS version: && makensis /VERSION
git --version

echo.
echo Next steps:
echo 1. Run configure.bat to set up build environment
echo 2. Run build-all.bat to build the complete project
echo.

REM 询问是否运行配置
set /p RUN_CONFIG="Do you want to run the configuration now? (y/n): "
if /i "%RUN_CONFIG%"=="y" (
    call configure.bat
)

pause