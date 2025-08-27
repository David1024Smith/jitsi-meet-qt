@echo off
REM Jitsi Meet Qt - CPack打包脚本
REM 使用CPack创建Windows安装程序

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Package Creation Script
echo ========================================

set BUILD_DIR=build
set QT_DIR=C:\Qt\5.15.2\mingw81_64
set MINGW_DIR=C:\Qt\Tools\mingw810_64

REM 检查构建目录
if not exist "%BUILD_DIR%" (
    echo Error: Build directory not found. Please build the project first.
    echo Run: build-cmake.bat
    pause
    exit /b 1
)

REM 设置环境变量
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

cd %BUILD_DIR%

echo.
echo ========================================
echo Creating NSIS installer...
echo ========================================

REM 创建NSIS安装程序
cpack -G NSIS

if errorlevel 1 (
    echo Error: NSIS package creation failed
    echo Make sure NSIS is installed and in PATH
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Creating ZIP package...
echo ========================================

REM 创建ZIP包
cpack -G ZIP

if errorlevel 1 (
    echo Error: ZIP package creation failed
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Package creation completed!
echo ========================================

REM 显示创建的包
echo Created packages:
for %%f in ("%BUILD_DIR%\*.exe") do echo   NSIS Installer: %%f
for %%f in ("%BUILD_DIR%\*.zip") do echo   ZIP Package: %%f

echo.
echo Installation packages are ready for distribution.

pause