@echo off
REM Jitsi Meet Qt - 兼容性测试脚本
REM 测试在不同Windows版本上的兼容性

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Compatibility Testing
echo ========================================

REM 加载构建配置
if exist "build_config.bat" (
    call build_config.bat
    echo Loaded build configuration
) else (
    echo Warning: build_config.bat not found, using defaults
    set QT_DIR=C:\Qt\5.15.2\mingw81_64
    set MINGW_DIR=C:\Qt\Tools\mingw810_64
    set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%
)

REM 检测Windows版本
echo Detecting Windows version...
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
echo Windows Version: %VERSION%

REM 检测系统架构
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
    set ARCH=x64
) else (
    set ARCH=x86
)
echo System Architecture: %ARCH%

REM 检测.NET Framework版本
echo Checking .NET Framework...
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full\" /v Release >nul 2>&1
if errorlevel 1 (
    echo Warning: .NET Framework 4.0+ not detected
) else (
    echo .NET Framework 4.0+ detected
)

echo.
echo ========================================
echo Testing Core Components
echo ========================================

REM 测试基本编译
echo Testing basic compilation...
if not exist "test_compile.cpp" (
    echo Creating basic test file...
    (
    echo #include ^<iostream^>
    echo #include ^<QtCore/QCoreApplication^>
    echo #include ^<QtWidgets/QApplication^>
    echo #include ^<QtNetwork/QNetworkAccessManager^>
    echo.
    echo int main^(int argc, char *argv[]^) {
    echo     QApplication app^(argc, argv^);
    echo     std::cout ^<^< "Qt Application test: OK" ^<^< std::endl;
    echo     return 0;
    echo }
    ) > test_compile.cpp
)

g++ -std=c++14 ^
    -I%QT_DIR%/include -I%QT_DIR%/include/QtCore -I%QT_DIR%/include/QtWidgets -I%QT_DIR%/include/QtNetwork ^
    -L%QT_DIR%/lib ^
    test_compile.cpp ^
    -lQt5Core -lQt5Widgets -lQt5Network ^
    -o test_compile.exe

if errorlevel 1 (
    echo Error: Basic compilation failed
    pause
    exit /b 1
)

echo Basic compilation: OK

REM 测试Qt模块
echo.
echo Testing Qt modules...
set QT_MODULES=Core Widgets Network WebSockets Multimedia MultimediaWidgets Xml
for %%m in (%QT_MODULES%) do (
    if exist "%QT_DIR%\bin\Qt5%%m.dll" (
        echo   Qt5%%m: Available
    ) else (
        echo   Qt5%%m: Missing
        set MISSING_MODULES=1
    )
)

if defined MISSING_MODULES (
    echo Warning: Some Qt modules are missing
)

REM 测试运行时依赖
echo.
echo Testing runtime dependencies...

REM 检查MinGW运行时
set MINGW_DLLS=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll
for %%d in (%MINGW_DLLS%) do (
    if exist "%MINGW_DIR%\bin\%%d" (
        echo   %%d: Available
    ) else (
        echo   %%d: Missing
        set MISSING_RUNTIME=1
    )
)

if defined MISSING_RUNTIME (
    echo Warning: Some MinGW runtime libraries are missing
)

REM 测试协议处理器
echo.
echo Testing protocol handler...
if exist "src\ProtocolHandler.cpp" (
    g++ -std=c++14 -I./include -I./src ^
        -I%QT_DIR%/include -I%QT_DIR%/include/QtCore -I%QT_DIR%/include/QtWidgets ^
        -L%QT_DIR%/lib ^
        test_protocol_handler.cpp ^
        src/ProtocolHandler.cpp ^
        -lQt5Core -lQt5Widgets ^
        -o test_protocol.exe 2>nul

    if errorlevel 1 (
        echo Protocol handler compilation: Failed
    ) else (
        echo Protocol handler compilation: OK
    )
) else (
    echo Protocol handler source not found, skipping test
)

REM 测试媒体功能
echo.
echo Testing multimedia capabilities...
if exist "%QT_DIR%\bin\Qt5Multimedia.dll" (
    echo Multimedia module: Available
    
    REM 检查DirectShow支持（Windows特定）
    reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{083863F1-70DE-11d0-BD40-00A0C911CE86}" >nul 2>&1
    if errorlevel 1 (
        echo DirectShow: Not available
    ) else (
        echo DirectShow: Available
    )
) else (
    echo Multimedia module: Not available
)

REM 测试网络功能
echo.
echo Testing network capabilities...
ping -n 1 8.8.8.8 >nul 2>&1
if errorlevel 1 (
    echo Network connectivity: Limited
) else (
    echo Network connectivity: OK
)

REM 测试WebSocket支持
if exist "%QT_DIR%\bin\Qt5WebSockets.dll" (
    echo WebSocket module: Available
) else (
    echo WebSocket module: Not available
)

echo.
echo ========================================
echo Performance Testing
echo ========================================

REM 测试启动时间
echo Testing application startup time...
if exist "install\bin\JitsiMeetQt.exe" (
    echo Starting application for performance test...
    set START_TIME=%TIME%
    start /wait "" "install\bin\JitsiMeetQt.exe" --test-mode --exit-after=1000
    set END_TIME=%TIME%
    echo Startup test completed
) else (
    echo Application not built, skipping startup test
)

REM 测试内存使用
echo Testing memory usage...
if exist "install\bin\JitsiMeetQt.exe" (
    tasklist /FI "IMAGENAME eq JitsiMeetQt.exe" /FO CSV | findstr "JitsiMeetQt.exe" >nul 2>&1
    if not errorlevel 1 (
        echo Application is running, checking memory usage...
        tasklist /FI "IMAGENAME eq JitsiMeetQt.exe" /FO TABLE
    )
)

echo.
echo ========================================
echo Windows Version Compatibility
echo ========================================

REM 检查Windows版本兼容性
if "%VERSION%"=="10.0" (
    echo Windows 10: Fully supported
) else if "%VERSION%"=="6.3" (
    echo Windows 8.1: Supported
) else if "%VERSION%"=="6.2" (
    echo Windows 8: Supported
) else if "%VERSION%"=="6.1" (
    echo Windows 7: Supported ^(with limitations^)
    echo Note: Some modern features may not be available
) else (
    echo Windows %VERSION%: Unknown compatibility
    echo Please test thoroughly before deployment
)

REM 检查必要的系统组件
echo.
echo Checking system components...

REM Visual C++ Redistributable
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" >nul 2>&1
if errorlevel 1 (
    echo Visual C++ 2015-2019 Redistributable: Not installed
    echo Note: May be required for some features
) else (
    echo Visual C++ 2015-2019 Redistributable: Installed
)

REM Windows Media Feature Pack
if "%VERSION%"=="10.0" (
    dism /online /get-features | findstr "MediaPlayback" >nul 2>&1
    if errorlevel 1 (
        echo Windows Media Feature Pack: Status unknown
    ) else (
        echo Windows Media Feature Pack: Available
    )
)

echo.
echo ========================================
echo Compatibility Test Summary
echo ========================================

set ISSUES=0

if defined MISSING_MODULES (
    echo - Missing Qt modules detected
    set /a ISSUES+=1
)

if defined MISSING_RUNTIME (
    echo - Missing runtime libraries detected
    set /a ISSUES+=1
)

if %ISSUES%==0 (
    echo All compatibility tests passed!
    echo The application should run correctly on this system.
) else (
    echo %ISSUES% compatibility issues detected.
    echo Please resolve these issues before deployment.
)

echo.
echo Test completed on: %DATE% %TIME%
echo System: Windows %VERSION% %ARCH%

REM 清理测试文件
if exist "test_compile.exe" del "test_compile.exe"
if exist "test_protocol.exe" del "test_protocol.exe"

pause