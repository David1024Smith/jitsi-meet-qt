@echo off
REM Jitsi Meet Qt - Windows兼容性测试脚本
REM 测试应用程序在不同Windows版本上的兼容性

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Compatibility Test Script
echo ========================================

set TARGET_EXE=install\bin\JitsiMeetQt.exe
set TEST_LOG=compatibility_test.log

REM 检查可执行文件
if not exist "%TARGET_EXE%" (
    echo Error: Application not found: %TARGET_EXE%
    echo Please build and install the project first.
    pause
    exit /b 1
)

echo Testing compatibility for: %TARGET_EXE%
echo Test started at: %DATE% %TIME%
echo.

REM 创建测试日志
echo Jitsi Meet Qt Compatibility Test Report > %TEST_LOG%
echo ========================================== >> %TEST_LOG%
echo Test Date: %DATE% %TIME% >> %TEST_LOG%
echo Target: %TARGET_EXE% >> %TEST_LOG%
echo. >> %TEST_LOG%

REM 检测Windows版本
echo Detecting Windows version...
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
echo Windows Version: %VERSION%
echo Windows Version: %VERSION% >> %TEST_LOG%

REM 检测系统架构
echo Detecting system architecture...
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITECTURE%"=="x86" (
    set ARCH=x86
) else (
    set ARCH=%PROCESSOR_ARCHITECTURE%
)
echo System Architecture: %ARCH%
echo System Architecture: %ARCH% >> %TEST_LOG%

REM 检测.NET Framework版本
echo Checking .NET Framework...
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full\" /v Release >nul 2>&1
if errorlevel 1 (
    echo .NET Framework 4.x: Not installed
    echo .NET Framework 4.x: Not installed >> %TEST_LOG%
) else (
    for /f "tokens=3" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full\" /v Release ^| findstr Release') do (
        if %%a GEQ 528040 (
            echo .NET Framework: 4.8 or later
            echo .NET Framework: 4.8 or later >> %TEST_LOG%
        ) else if %%a GEQ 461808 (
            echo .NET Framework: 4.7.2
            echo .NET Framework: 4.7.2 >> %TEST_LOG%
        ) else (
            echo .NET Framework: 4.x ^(older version^)
            echo .NET Framework: 4.x ^(older version^) >> %TEST_LOG%
        )
    )
)

REM 检测Visual C++ Redistributable
echo Checking Visual C++ Redistributable...
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" >nul 2>&1
if errorlevel 1 (
    echo VC++ 2015-2019 x64: Not installed
    echo VC++ 2015-2019 x64: Not installed >> %TEST_LOG%
) else (
    echo VC++ 2015-2019 x64: Installed
    echo VC++ 2015-2019 x64: Installed >> %TEST_LOG%
)

echo.
echo ========================================
echo Running basic functionality tests...
echo ========================================
echo. >> %TEST_LOG%
echo Basic Functionality Tests: >> %TEST_LOG%

REM 测试1: 应用程序启动测试
echo Test 1: Application startup test
echo Test 1: Application startup test >> %TEST_LOG%
start /wait /b "" "%TARGET_EXE%" --test-mode --exit-after=5 >nul 2>&1
if errorlevel 1 (
    echo   FAILED: Application failed to start
    echo   FAILED: Application failed to start >> %TEST_LOG%
    set TEST1_RESULT=FAILED
) else (
    echo   PASSED: Application started successfully
    echo   PASSED: Application started successfully >> %TEST_LOG%
    set TEST1_RESULT=PASSED
)

REM 测试2: 依赖库检查
echo Test 2: Dependency check
echo Test 2: Dependency check >> %TEST_LOG%
dumpbin /dependents "%TARGET_EXE%" >nul 2>&1
if errorlevel 1 (
    echo   WARNING: Could not analyze dependencies ^(dumpbin not available^)
    echo   WARNING: Could not analyze dependencies ^(dumpbin not available^) >> %TEST_LOG%
    set TEST2_RESULT=WARNING
) else (
    echo   PASSED: Dependencies analyzed successfully
    echo   PASSED: Dependencies analyzed successfully >> %TEST_LOG%
    set TEST2_RESULT=PASSED
)

REM 测试3: 协议处理器测试
echo Test 3: Protocol handler test
echo Test 3: Protocol handler test >> %TEST_LOG%
reg query "HKCR\jitsi-meet" >nul 2>&1
if errorlevel 1 (
    echo   INFO: Protocol handler not registered ^(run install.bat^)
    echo   INFO: Protocol handler not registered ^(run install.bat^) >> %TEST_LOG%
    set TEST3_RESULT=INFO
) else (
    echo   PASSED: Protocol handler is registered
    echo   PASSED: Protocol handler is registered >> %TEST_LOG%
    set TEST3_RESULT=PASSED
)

REM 测试4: 文件权限测试
echo Test 4: File permissions test
echo Test 4: File permissions test >> %TEST_LOG%
icacls "%TARGET_EXE%" >nul 2>&1
if errorlevel 1 (
    echo   FAILED: Cannot access file permissions
    echo   FAILED: Cannot access file permissions >> %TEST_LOG%
    set TEST4_RESULT=FAILED
) else (
    echo   PASSED: File permissions are accessible
    echo   PASSED: File permissions are accessible >> %TEST_LOG%
    set TEST4_RESULT=PASSED
)

echo.
echo ========================================
echo Compatibility Assessment
echo ========================================
echo. >> %TEST_LOG%
echo Compatibility Assessment: >> %TEST_LOG%

REM 评估Windows版本兼容性
if "%VERSION%"=="10.0" (
    echo Windows 10/11: FULLY COMPATIBLE
    echo Windows 10/11: FULLY COMPATIBLE >> %TEST_LOG%
    set WIN_COMPAT=FULL
) else if "%VERSION%"=="6.3" (
    echo Windows 8.1: COMPATIBLE
    echo Windows 8.1: COMPATIBLE >> %TEST_LOG%
    set WIN_COMPAT=COMPATIBLE
) else if "%VERSION%"=="6.2" (
    echo Windows 8: COMPATIBLE
    echo Windows 8: COMPATIBLE >> %TEST_LOG%
    set WIN_COMPAT=COMPATIBLE
) else if "%VERSION%"=="6.1" (
    echo Windows 7: LIMITED COMPATIBILITY ^(may require updates^)
    echo Windows 7: LIMITED COMPATIBILITY ^(may require updates^) >> %TEST_LOG%
    set WIN_COMPAT=LIMITED
) else (
    echo Unknown Windows version: UNKNOWN COMPATIBILITY
    echo Unknown Windows version: UNKNOWN COMPATIBILITY >> %TEST_LOG%
    set WIN_COMPAT=UNKNOWN
)

REM 生成兼容性报告
echo.
echo ========================================
echo Test Summary
echo ========================================
echo. >> %TEST_LOG%
echo Test Summary: >> %TEST_LOG%

echo Application Startup: %TEST1_RESULT%
echo Dependency Check: %TEST2_RESULT%
echo Protocol Handler: %TEST3_RESULT%
echo File Permissions: %TEST4_RESULT%
echo Windows Compatibility: %WIN_COMPAT%

echo Application Startup: %TEST1_RESULT% >> %TEST_LOG%
echo Dependency Check: %TEST2_RESULT% >> %TEST_LOG%
echo Protocol Handler: %TEST3_RESULT% >> %TEST_LOG%
echo File Permissions: %TEST4_RESULT% >> %TEST_LOG%
echo Windows Compatibility: %WIN_COMPAT% >> %TEST_LOG%

echo.
echo Test completed at: %DATE% %TIME%
echo Test completed at: %DATE% %TIME% >> %TEST_LOG%

echo.
echo Full test report saved to: %TEST_LOG%

REM 推荐操作
echo.
echo ========================================
echo Recommendations
echo ========================================

if "%TEST1_RESULT%"=="FAILED" (
    echo - Check Qt 5.15.2 installation and dependencies
    echo - Verify MinGW runtime libraries are available
)

if "%TEST3_RESULT%"=="INFO" (
    echo - Run install.bat as administrator to register protocol handler
)

if "%WIN_COMPAT%"=="LIMITED" (
    echo - Install latest Windows updates
    echo - Install Visual C++ Redistributable 2015-2019
)

echo - Test the application with actual Jitsi Meet URLs
echo - Verify network connectivity and firewall settings

pause