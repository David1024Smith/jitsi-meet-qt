@echo off
REM Jitsi Meet Qt - 代码签名脚本
REM 使用数字证书对可执行文件进行签名

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Code Signing Script
echo ========================================

REM 配置签名参数
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\signtool.exe"
set CERT_FILE=certificate.p12
set CERT_PASSWORD_FILE=cert_password.txt
set TIMESTAMP_URL=http://timestamp.digicert.com

REM 要签名的文件
set TARGET_EXE=install\bin\JitsiMeetQt.exe
set INSTALLER_EXE=build\JitsiMeetQt-*.exe

REM 检查signtool是否存在
if not exist %SIGNTOOL% (
    echo Error: SignTool not found at %SIGNTOOL%
    echo Please install Windows SDK or update SIGNTOOL path
    echo.
    echo Alternative locations to check:
    echo - C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe
    echo - C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe
    pause
    exit /b 1
)

REM 检查证书文件
if not exist "%CERT_FILE%" (
    echo Warning: Certificate file not found: %CERT_FILE%
    echo.
    echo To enable code signing:
    echo 1. Obtain a code signing certificate from a trusted CA
    echo 2. Export it as a .p12 file and place it as %CERT_FILE%
    echo 3. Create %CERT_PASSWORD_FILE% with the certificate password
    echo.
    echo Skipping code signing...
    pause
    goto :eof
)

REM 读取证书密码
if not exist "%CERT_PASSWORD_FILE%" (
    echo Error: Certificate password file not found: %CERT_PASSWORD_FILE%
    echo Please create this file with your certificate password
    pause
    exit /b 1
)

set /p CERT_PASSWORD=<%CERT_PASSWORD_FILE%

echo Certificate file: %CERT_FILE%
echo Timestamp URL: %TIMESTAMP_URL%

REM 签名主执行文件
if exist "%TARGET_EXE%" (
    echo.
    echo Signing main executable: %TARGET_EXE%
    %SIGNTOOL% sign /f "%CERT_FILE%" /p "%CERT_PASSWORD%" /t "%TIMESTAMP_URL%" /v "%TARGET_EXE%"
    
    if errorlevel 1 (
        echo Error: Failed to sign %TARGET_EXE%
        pause
        exit /b 1
    )
    
    echo Successfully signed: %TARGET_EXE%
) else (
    echo Warning: Main executable not found: %TARGET_EXE%
)

REM 签名安装程序
for %%f in (%INSTALLER_EXE%) do (
    if exist "%%f" (
        echo.
        echo Signing installer: %%f
        %SIGNTOOL% sign /f "%CERT_FILE%" /p "%CERT_PASSWORD%" /t "%TIMESTAMP_URL%" /v "%%f"
        
        if errorlevel 1 (
            echo Error: Failed to sign %%f
            pause
            exit /b 1
        )
        
        echo Successfully signed: %%f
    )
)

echo.
echo ========================================
echo Verifying signatures...
echo ========================================

REM 验证签名
if exist "%TARGET_EXE%" (
    echo Verifying: %TARGET_EXE%
    %SIGNTOOL% verify /pa /v "%TARGET_EXE%"
)

for %%f in (%INSTALLER_EXE%) do (
    if exist "%%f" (
        echo Verifying: %%f
        %SIGNTOOL% verify /pa /v "%%f"
    )
)

echo.
echo ========================================
echo Code signing completed!
echo ========================================

pause