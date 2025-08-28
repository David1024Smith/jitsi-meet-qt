@echo off
echo Windows Compatibility Testing for Jitsi Meet Qt
echo ===============================================

REM Get Windows version information
echo Detecting Windows version...
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
echo Windows version: %VERSION%

REM Check if executable exists
set EXECUTABLE_PATH="install\bin\JitsiMeetQt.exe"
if not exist %EXECUTABLE_PATH% (
    echo Error: Executable not found at %EXECUTABLE_PATH%
    echo Please build and install the application first.
    pause
    exit /b 1
)

REM Test basic executable launch
echo Testing basic executable launch...
start /wait "" %EXECUTABLE_PATH% --test-mode --exit-after=5

if %ERRORLEVEL% neq 0 (
    echo Error: Application failed to launch properly.
    pause
    exit /b 1
)

REM Check dependencies
echo Checking dependencies...
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM Test Qt libraries
echo Testing Qt library dependencies...
where Qt6Core.dll >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Warning: Qt6Core.dll not found in PATH
)

where Qt6Widgets.dll >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Warning: Qt6Widgets.dll not found in PATH
)

where Qt6Network.dll >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Warning: Qt6Network.dll not found in PATH
)

REM Test protocol registration
echo Testing protocol registration...
reg query "HKEY_CURRENT_USER\Software\Classes\jitsi-meet" >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Warning: jitsi-meet:// protocol not registered
) else (
    echo Protocol registration: OK
)

REM Test configuration directory
echo Testing configuration directory...
if not exist "%APPDATA%\JitsiMeetQt" (
    echo Info: Configuration directory will be created on first run
) else (
    echo Configuration directory: OK
)

REM Performance test
echo Running basic performance test...
powershell -Command "Measure-Command { Start-Process -FilePath %EXECUTABLE_PATH% -ArgumentList '--test-mode', '--exit-after=3' -Wait }" > temp_perf.txt
for /f "tokens=*" %%i in (temp_perf.txt) do echo Startup time: %%i
del temp_perf.txt

echo.
echo Compatibility testing completed!
echo Check the output above for any warnings or errors.
pause