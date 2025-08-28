@echo off
echo Code Signing for Jitsi Meet Qt
echo ==============================

REM Configuration - Update these paths according to your setup
set SIGNTOOL_PATH="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"
set CERTIFICATE_PATH="path\to\your\certificate.p12"
set TIMESTAMP_URL="http://timestamp.digicert.com"

REM Check if signtool exists
if not exist %SIGNTOOL_PATH% (
    echo Error: SignTool not found at %SIGNTOOL_PATH%
    echo Please install Windows SDK or update the path.
    pause
    exit /b 1
)

REM Check if executable exists
set EXECUTABLE_PATH="install\bin\JitsiMeetQt.exe"
if not exist %EXECUTABLE_PATH% (
    echo Error: Executable not found at %EXECUTABLE_PATH%
    echo Please build and install the application first.
    pause
    exit /b 1
)

REM Check if certificate exists
if not exist %CERTIFICATE_PATH% (
    echo Error: Certificate not found at %CERTIFICATE_PATH%
    echo Please update the certificate path or obtain a code signing certificate.
    echo Skipping code signing...
    pause
    exit /b 0
)

REM Prompt for certificate password
set /p CERT_PASSWORD="Enter certificate password: "

REM Sign the executable
echo Signing executable...
%SIGNTOOL_PATH% sign /f %CERTIFICATE_PATH% /p %CERT_PASSWORD% /t %TIMESTAMP_URL% /v %EXECUTABLE_PATH%

if %ERRORLEVEL% neq 0 (
    echo Code signing failed!
    pause
    exit /b 1
)

REM Verify the signature
echo Verifying signature...
%SIGNTOOL_PATH% verify /pa /v %EXECUTABLE_PATH%

if %ERRORLEVEL% neq 0 (
    echo Signature verification failed!
    pause
    exit /b 1
)

echo Code signing completed successfully!
pause