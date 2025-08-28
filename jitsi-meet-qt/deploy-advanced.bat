@echo off
REM Jitsi Meet Qt - 高级部署脚本
REM 支持多种部署模式和目标环境

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Advanced Deployment
echo ========================================

REM 部署模式
set DEPLOY_MODE=full
set TARGET_ENV=production
set INCLUDE_DEBUG=0
set CREATE_PORTABLE=1
set CREATE_INSTALLER=1

REM 处理命令行参数
:parse_args
if "%1"=="--mode" (
    set DEPLOY_MODE=%2
    shift
    shift
    goto parse_args
)
if "%1"=="--env" (
    set TARGET_ENV=%2
    shift
    shift
    goto parse_args
)
if "%1"=="--debug" (
    set INCLUDE_DEBUG=1
    shift
    goto parse_args
)
if "%1"=="--portable-only" (
    set CREATE_PORTABLE=1
    set CREATE_INSTALLER=0
    shift
    goto parse_args
)
if "%1"=="--installer-only" (
    set CREATE_PORTABLE=0
    set CREATE_INSTALLER=1
    shift
    goto parse_args
)
if "%1"=="--help" (
    echo Usage: deploy-advanced.bat [options]
    echo.
    echo Options:
    echo   --mode ^<full^|minimal^|debug^>     Deployment mode
    echo   --env ^<production^|staging^|dev^>   Target environment
    echo   --debug                        Include debug symbols
    echo   --portable-only                Create only portable package
    echo   --installer-only               Create only installer
    echo   --help                         Show this help
    echo.
    echo Deployment modes:
    echo   full     - Complete deployment with all features
    echo   minimal  - Minimal deployment without optional components
    echo   debug    - Debug deployment with symbols and tools
    echo.
    pause
    exit /b 0
)
if "%1" neq "" (
    shift
    goto parse_args
)

echo Deployment Mode: %DEPLOY_MODE%
echo Target Environment: %TARGET_ENV%
echo Include Debug Info: %INCLUDE_DEBUG%
echo Create Portable: %CREATE_PORTABLE%
echo Create Installer: %CREATE_INSTALLER%

REM 加载构建配置
if exist "build_config.bat" (
    call build_config.bat
    echo Loaded build configuration
) else (
    echo Error: build_config.bat not found
    echo Please run configure.bat first
    pause
    exit /b 1
)

REM 设置部署目录
set DEPLOY_BASE=deploy
set DEPLOY_DIR=%DEPLOY_BASE%\%DEPLOY_MODE%
set PACKAGE_DIR=packages

REM 检查构建是否存在
if not exist "install\bin\JitsiMeetQt.exe" (
    echo Error: Application not found. Please build the project first.
    echo Run: build-cmake.bat
    pause
    exit /b 1
)

echo.
echo ========================================
echo Preparing deployment...
echo ========================================

REM 清理并创建部署目录
if exist %DEPLOY_DIR% rmdir /s /q %DEPLOY_DIR%
mkdir %DEPLOY_DIR%
mkdir %DEPLOY_DIR%\bin
mkdir %DEPLOY_DIR%\lib
mkdir %DEPLOY_DIR%\plugins
mkdir %DEPLOY_DIR%\translations
mkdir %DEPLOY_DIR%\resources

REM 复制主应用程序
echo Copying application files...
copy "install\bin\JitsiMeetQt.exe" "%DEPLOY_DIR%\bin\" >nul
if errorlevel 1 (
    echo Error: Failed to copy main executable
    pause
    exit /b 1
)

REM 复制Qt库
echo Deploying Qt libraries...
set QT_LIBS=Qt5Core Qt5Gui Qt5Widgets Qt5Network Qt5WebSockets Qt5Multimedia Qt5MultimediaWidgets Qt5Xml
for %%lib in (%QT_LIBS%) do (
    if exist "%QT_DIR%\bin\%%lib.dll" (
        copy "%QT_DIR%\bin\%%lib.dll" "%DEPLOY_DIR%\bin\" >nul
        echo   %%lib.dll: OK
    ) else (
        echo   %%lib.dll: Missing
    )
)

REM 复制Qt平台插件
echo Copying Qt platform plugins...
if exist "%QT_DIR%\plugins\platforms" (
    xcopy /E /I /Y "%QT_DIR%\plugins\platforms" "%DEPLOY_DIR%\plugins\platforms\" >nul
)

REM 复制Qt图像格式插件
echo Copying Qt image format plugins...
if exist "%QT_DIR%\plugins\imageformats" (
    xcopy /E /I /Y "%QT_DIR%\plugins\imageformats" "%DEPLOY_DIR%\plugins\imageformats\" >nul
)

REM 复制Qt多媒体插件
if "%DEPLOY_MODE%"=="full" (
    echo Copying Qt multimedia plugins...
    if exist "%QT_DIR%\plugins\mediaservice" (
        xcopy /E /I /Y "%QT_DIR%\plugins\mediaservice" "%DEPLOY_DIR%\plugins\mediaservice\" >nul
    )
    if exist "%QT_DIR%\plugins\audio" (
        xcopy /E /I /Y "%QT_DIR%\plugins\audio" "%DEPLOY_DIR%\plugins\audio\" >nul
    )
)

REM 复制MinGW运行时
echo Copying MinGW runtime libraries...
set MINGW_LIBS=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll
for %%lib in (%MINGW_LIBS%) do (
    if exist "%MINGW_DIR%\bin\%%lib" (
        copy "%MINGW_DIR%\bin\%%lib" "%DEPLOY_DIR%\bin\" >nul
        echo   %%lib: OK
    ) else (
        echo   %%lib: Missing
    )
)

REM 复制OpenSSL库（如果存在）
if exist "%QT_DIR%\bin\libssl-1_1-x64.dll" (
    echo Copying OpenSSL libraries...
    copy "%QT_DIR%\bin\libssl-1_1-x64.dll" "%DEPLOY_DIR%\bin\" >nul
    copy "%QT_DIR%\bin\libcrypto-1_1-x64.dll" "%DEPLOY_DIR%\bin\" >nul
)

REM 复制翻译文件
echo Copying translation files...
if exist "translations\*.qm" (
    copy "translations\*.qm" "%DEPLOY_DIR%\translations\" >nul
)

REM 复制资源文件
echo Copying resource files...
if exist "resources\icons" (
    xcopy /E /I /Y "resources\icons" "%DEPLOY_DIR%\resources\icons\" >nul
)

REM 复制配置文件
if "%TARGET_ENV%"=="production" (
    echo Creating production configuration...
    (
    echo [General]
    echo ServerUrl=https://meet.jit.si
    echo AutoUpdate=true
    echo LogLevel=Warning
    echo.
    echo [UI]
    echo Theme=auto
    echo Language=auto
    ) > "%DEPLOY_DIR%\config.ini"
) else if "%TARGET_ENV%"=="staging" (
    echo Creating staging configuration...
    (
    echo [General]
    echo ServerUrl=https://staging.meet.jit.si
    echo AutoUpdate=false
    echo LogLevel=Info
    echo.
    echo [UI]
    echo Theme=auto
    echo Language=auto
    ) > "%DEPLOY_DIR%\config.ini"
) else (
    echo Creating development configuration...
    (
    echo [General]
    echo ServerUrl=https://localhost:8443
    echo AutoUpdate=false
    echo LogLevel=Debug
    echo.
    echo [UI]
    echo Theme=auto
    echo Language=auto
    ) > "%DEPLOY_DIR%\config.ini"
)

REM 创建qt.conf文件
echo Creating Qt configuration...
(
echo [Paths]
echo Plugins = plugins
echo Translations = translations
) > "%DEPLOY_DIR%\bin\qt.conf"

REM 包含调试信息
if %INCLUDE_DEBUG%==1 (
    echo Including debug information...
    if exist "install\bin\JitsiMeetQt.pdb" (
        copy "install\bin\JitsiMeetQt.pdb" "%DEPLOY_DIR%\bin\" >nul
    )
    
    REM 复制调试版本的Qt库
    set QT_DEBUG_LIBS=Qt5Cored Qt5Guid Qt5Widgetsd Qt5Networkd Qt5WebSocketsd Qt5Multimediad Qt5MultimediaWidgetsd Qt5Xmld
    for %%lib in (%QT_DEBUG_LIBS%) do (
        if exist "%QT_DIR%\bin\%%lib.dll" (
            copy "%QT_DIR%\bin\%%lib.dll" "%DEPLOY_DIR%\bin\" >nul
        )
    )
)

REM 创建启动脚本
echo Creating launcher scripts...
(
echo @echo off
echo cd /d "%%~dp0"
echo set QT_PLUGIN_PATH=%%~dp0plugins
echo set QT_QPA_PLATFORM_PLUGIN_PATH=%%~dp0plugins\platforms
echo start "" "bin\JitsiMeetQt.exe" %%*
) > "%DEPLOY_DIR%\JitsiMeetQt.bat"

REM 创建调试启动脚本
if %INCLUDE_DEBUG%==1 (
    (
    echo @echo off
    echo cd /d "%%~dp0"
    echo set QT_PLUGIN_PATH=%%~dp0plugins
    echo set QT_QPA_PLATFORM_PLUGIN_PATH=%%~dp0plugins\platforms
    echo set QT_LOGGING_RULES=*.debug=true
    echo "bin\JitsiMeetQt.exe" --debug %%*
    echo pause
    ) > "%DEPLOY_DIR%\JitsiMeetQt-Debug.bat"
)

REM 创建卸载脚本
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
echo REM Remove start menu shortcut
echo del "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\Jitsi Meet Qt.lnk" ^>nul 2^>^&1
echo.
echo echo Uninstall completed.
echo pause
) > "%DEPLOY_DIR%\uninstall.bat"

REM 创建安装脚本
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
echo powershell -Command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%%USERPROFILE%%\Desktop\Jitsi Meet Qt.lnk'^); $Shortcut.TargetPath = '%%~dp0bin\JitsiMeetQt.exe'; $Shortcut.WorkingDirectory = '%%~dp0'; $Shortcut.Save(^)"
echo.
echo REM Create start menu shortcut
echo powershell -Command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\Jitsi Meet Qt.lnk'^); $Shortcut.TargetPath = '%%~dp0bin\JitsiMeetQt.exe'; $Shortcut.WorkingDirectory = '%%~dp0'; $Shortcut.Save(^)"
echo.
echo echo Installation completed.
echo echo You can now use jitsi-meet:// URLs to launch the application.
echo pause
) > "%DEPLOY_DIR%\install.bat"

REM 复制文档
echo Copying documentation...
if exist "README.md" copy "README.md" "%DEPLOY_DIR%\" >nul
if exist "LICENSE" copy "LICENSE" "%DEPLOY_DIR%\" >nul

REM 创建版本信息文件
echo Creating version information...
for /f "tokens=3" %%i in ('findstr "VERSION" CMakeLists.txt ^| findstr "project"') do set VERSION=%%i
set VERSION=%VERSION:~0,-1%
if "%VERSION%"=="" set VERSION=1.0.0

(
echo Jitsi Meet Qt v%VERSION%
echo Build Date: %DATE% %TIME%
echo Deployment Mode: %DEPLOY_MODE%
echo Target Environment: %TARGET_ENV%
echo.
echo System Requirements:
echo - Windows 7 SP1 or later
echo - 2 GB RAM minimum
echo - 100 MB disk space
echo - Internet connection for video calls
echo.
echo For support, visit: https://github.com/jitsi/jitsi-meet-qt
) > "%DEPLOY_DIR%\VERSION.txt"

echo.
echo ========================================
echo Creating packages...
echo ========================================

if not exist %PACKAGE_DIR% mkdir %PACKAGE_DIR%

REM 创建便携版ZIP包
if %CREATE_PORTABLE%==1 (
    set PORTABLE_NAME=JitsiMeetQt-v%VERSION%-%DEPLOY_MODE%-Windows-x64-Portable
    echo Creating portable package: !PORTABLE_NAME!.zip
    
    powershell -Command "Compress-Archive -Path '%DEPLOY_DIR%\*' -DestinationPath '%PACKAGE_DIR%\!PORTABLE_NAME!.zip' -Force"
    
    if errorlevel 1 (
        echo Error: Failed to create portable package
    ) else (
        echo Portable package created successfully
    )
)

REM 创建安装程序
if %CREATE_INSTALLER%==1 (
    if %NSIS_AVAILABLE%==1 (
        echo Creating NSIS installer...
        cd build
        cpack -G NSIS -C Release
        cd ..
        
        REM 移动安装程序到packages目录
        for %%f in ("build\JitsiMeetQt-*.exe") do (
            set INSTALLER_NAME=JitsiMeetQt-v%VERSION%-%DEPLOY_MODE%-Windows-x64-Setup.exe
            move "%%f" "%PACKAGE_DIR%\!INSTALLER_NAME!" >nul 2>&1
            if not errorlevel 1 echo Installer created: !INSTALLER_NAME!
        )
    ) else (
        echo NSIS not available, skipping installer creation
    )
)

echo.
echo ========================================
echo Deployment Summary
echo ========================================

echo Deployment completed successfully!
echo.
echo Deployment directory: %DEPLOY_DIR%
echo Package directory: %PACKAGE_DIR%
echo.

if %CREATE_PORTABLE%==1 (
    echo Portable packages:
    for %%f in ("%PACKAGE_DIR%\*Portable*.zip") do echo   %%~nxf
)

if %CREATE_INSTALLER%==1 (
    echo Installer packages:
    for %%f in ("%PACKAGE_DIR%\*Setup*.exe") do echo   %%~nxf
)

echo.
echo Deployment mode: %DEPLOY_MODE%
echo Target environment: %TARGET_ENV%
echo Debug information: %INCLUDE_DEBUG%

REM 计算部署大小
for /f "usebackq" %%A in (`powershell -command "(Get-ChildItem '%DEPLOY_DIR%' -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB"`) do set DEPLOY_SIZE=%%A
echo Deployment size: %DEPLOY_SIZE% MB

echo.
echo To test the deployment:
echo 1. Extract the portable package or run the installer
echo 2. Run install.bat as administrator to register protocol handler
echo 3. Launch JitsiMeetQt.exe or use jitsi-meet:// URLs

REM 询问是否测试部署
set /p TEST_DEPLOY="Do you want to test the deployed application? (y/n): "
if /i "%TEST_DEPLOY%"=="y" (
    start "" "%DEPLOY_DIR%\bin\JitsiMeetQt.exe"
)

pause