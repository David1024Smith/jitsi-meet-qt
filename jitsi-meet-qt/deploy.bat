@echo off
REM Jitsi Meet Qt - 部署脚本
REM 创建可分发的应用程序包

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Deployment Script
echo ========================================

REM 设置路径
set QT_DIR=C:\Qt\5.15.2\mingw81_64
set MINGW_DIR=C:\Qt\Tools\mingw810_64
set BUILD_DIR=build
set INSTALL_DIR=install
set DEPLOY_DIR=deploy
set PACKAGE_DIR=package

REM 检查构建是否存在
if not exist "%INSTALL_DIR%\bin\JitsiMeetQt.exe" (
    echo Error: Application not found. Please build the project first.
    echo Run: build-cmake.bat
    pause
    exit /b 1
)

REM 设置环境变量
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

echo Creating deployment package...

REM 清理并创建部署目录
if exist %DEPLOY_DIR% rmdir /s /q %DEPLOY_DIR%
mkdir %DEPLOY_DIR%

REM 复制应用程序文件
echo Copying application files...
xcopy /E /I /Y "%INSTALL_DIR%\*" "%DEPLOY_DIR%\"

REM 使用windeployqt部署Qt依赖
echo Deploying Qt dependencies...
windeployqt.exe ^
    --qmldir . ^
    --verbose 2 ^
    --release ^
    --no-translations ^
    --no-system-d3d-compiler ^
    --no-opengl-sw ^
    --no-quick-import ^
    "%DEPLOY_DIR%\bin\JitsiMeetQt.exe"

if errorlevel 1 (
    echo Warning: windeployqt encountered issues, continuing...
)

REM 复制MinGW运行时库
echo Copying MinGW runtime libraries...
copy "%MINGW_DIR%\bin\libgcc_s_seh-1.dll" "%DEPLOY_DIR%\bin\" 2>nul
copy "%MINGW_DIR%\bin\libstdc++-6.dll" "%DEPLOY_DIR%\bin\" 2>nul
copy "%MINGW_DIR%\bin\libwinpthread-1.dll" "%DEPLOY_DIR%\bin\" 2>nul

REM 复制翻译文件
echo Copying translation files...
if exist "translations\*.qm" (
    if not exist "%DEPLOY_DIR%\translations" mkdir "%DEPLOY_DIR%\translations"
    copy "translations\*.qm" "%DEPLOY_DIR%\translations\" >nul 2>&1
)

REM 创建启动脚本
echo Creating launcher script...
(
echo @echo off
echo cd /d "%%~dp0"
echo start "" "bin\JitsiMeetQt.exe" %%*
) > "%DEPLOY_DIR%\JitsiMeetQt.bat"

REM 复制文档文件
echo Copying documentation...
copy "README.md" "%DEPLOY_DIR%\" >nul 2>&1

REM 创建卸载脚本
echo Creating uninstall script...
(
echo @echo off
echo echo Uninstalling Jitsi Meet Qt...
echo.
echo REM Remove protocol handler
echo reg delete "HKCR\jitsi-meet" /f ^>nul 2^>^&1
echo.
echo REM Remove desktop shortcut
echo del "%%USERPROFILE%%\Desktop\Jitsi Meet Qt.lnk" ^>nul 2^>^&1
echo.
echo echo Uninstall completed.
echo pause
) > "%DEPLOY_DIR%\uninstall.bat"

REM 创建安装脚本
echo Creating install script...
(
echo @echo off
echo echo Installing Jitsi Meet Qt...
echo.
echo REM Register protocol handler
echo reg add "HKCR\jitsi-meet" /ve /d "URL:Jitsi Meet Protocol" /f
echo reg add "HKCR\jitsi-meet" /v "URL Protocol" /f
echo reg add "HKCR\jitsi-meet\DefaultIcon" /ve /d "\"%%~dp0bin\JitsiMeetQt.exe\",0" /f
echo reg add "HKCR\jitsi-meet\shell\open\command" /ve /d "\"%%~dp0bin\JitsiMeetQt.exe\" \"%%1\"" /f
echo.
echo REM Create desktop shortcut
echo powershell -Command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%%USERPROFILE%%\Desktop\Jitsi Meet Qt.lnk'^); $Shortcut.TargetPath = '%%~dp0bin\JitsiMeetQt.exe'; $Shortcut.Save(^)"
echo.
echo echo Installation completed.
echo echo You can now use jitsi-meet:// URLs to launch the application.
echo pause
) > "%DEPLOY_DIR%\install.bat"

echo.
echo ========================================
echo Creating distribution package...
echo ========================================

REM 创建ZIP包
if exist %PACKAGE_DIR% rmdir /s /q %PACKAGE_DIR%
mkdir %PACKAGE_DIR%

REM 获取版本信息
for /f "tokens=3" %%i in ('findstr "VERSION" CMakeLists.txt ^| findstr "project"') do set VERSION=%%i
set VERSION=%VERSION:~0,-1%
if "%VERSION%"=="" set VERSION=1.0.0

set PACKAGE_NAME=JitsiMeetQt-v%VERSION%-Windows-x64

echo Creating package: %PACKAGE_NAME%.zip

REM 使用PowerShell创建ZIP文件
powershell -Command "Compress-Archive -Path '%DEPLOY_DIR%\*' -DestinationPath '%PACKAGE_DIR%\%PACKAGE_NAME%.zip' -Force"

if errorlevel 1 (
    echo Error: Failed to create ZIP package
    pause
    exit /b 1
)

echo.
echo ========================================
echo Deployment completed successfully!
echo ========================================
echo.
echo Deployment directory: %DEPLOY_DIR%
echo Package file: %PACKAGE_DIR%\%PACKAGE_NAME%.zip
echo.
echo To install on target machine:
echo 1. Extract the ZIP file
echo 2. Run install.bat as administrator
echo 3. Launch JitsiMeetQt.exe or use jitsi-meet:// URLs
echo.

REM 询问是否测试部署的应用程序
set /p TEST_APP="Do you want to test the deployed application? (y/n): "
if /i "%TEST_APP%"=="y" (
    start "" "%DEPLOY_DIR%\bin\JitsiMeetQt.exe"
)

pause