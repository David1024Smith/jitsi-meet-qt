@echo off
REM Jitsi Meet Qt - 增强代码签名脚本
REM 支持多种证书类型和签名方式

setlocal enabledelayedexpansion

echo ========================================
echo Jitsi Meet Qt - Enhanced Code Signing
echo ========================================

REM 签名配置
set CERT_TYPE=file
set CERT_FILE=certificate.p12
set CERT_PASSWORD_FILE=cert_password.txt
set CERT_STORE_NAME=My
set CERT_SUBJECT_NAME=
set TIMESTAMP_URL=http://timestamp.digicert.com
set TIMESTAMP_RFC3161=http://timestamp.digicert.com
set HASH_ALGORITHM=SHA256

REM 处理命令行参数
:parse_args
if "%1"=="--cert-file" (
    set CERT_TYPE=file
    set CERT_FILE=%2
    shift
    shift
    goto parse_args
)
if "%1"=="--cert-store" (
    set CERT_TYPE=store
    set CERT_SUBJECT_NAME=%2
    shift
    shift
    goto parse_args
)
if "%1"=="--timestamp" (
    set TIMESTAMP_URL=%2
    shift
    shift
    goto parse_args
)
if "%1"=="--help" (
    echo Usage: sign.bat [options]
    echo.
    echo Options:
    echo   --cert-file ^<file^>        Use certificate file ^(.p12/.pfx^)
    echo   --cert-store ^<subject^>    Use certificate from Windows store
    echo   --timestamp ^<url^>         Timestamp server URL
    echo   --help                    Show this help
    echo.
    echo Examples:
    echo   sign.bat --cert-file mycert.p12
    echo   sign.bat --cert-store "My Company Name"
    echo.
    pause
    exit /b 0
)
if "%1" neq "" (
    shift
    goto parse_args
)

REM 自动检测SignTool
set SIGNTOOL_FOUND=0
set SIGNTOOL_PATHS[0]=C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\signtool.exe
set SIGNTOOL_PATHS[1]=C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe
set SIGNTOOL_PATHS[2]=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe
set SIGNTOOL_PATHS[3]=C:\Program Files\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe

for /L %%i in (0,1,3) do (
    if exist "!SIGNTOOL_PATHS[%%i]!" (
        set SIGNTOOL="!SIGNTOOL_PATHS[%%i]!"
        set SIGNTOOL_FOUND=1
        echo Found SignTool at: !SIGNTOOL_PATHS[%%i]!
        goto :signtool_found
    )
)

:signtool_found
if %SIGNTOOL_FOUND%==0 (
    echo Error: SignTool not found in standard locations
    echo Please install Windows SDK
    echo.
    echo Checked locations:
    for /L %%i in (0,1,3) do echo   !SIGNTOOL_PATHS[%%i]!
    pause
    exit /b 1
)

echo Certificate Type: %CERT_TYPE%
echo Timestamp URL: %TIMESTAMP_URL%
echo Hash Algorithm: %HASH_ALGORITHM%

REM 要签名的文件列表
set FILES_TO_SIGN=
if exist "install\bin\JitsiMeetQt.exe" set FILES_TO_SIGN=%FILES_TO_SIGN% "install\bin\JitsiMeetQt.exe"
if exist "deploy\full\bin\JitsiMeetQt.exe" set FILES_TO_SIGN=%FILES_TO_SIGN% "deploy\full\bin\JitsiMeetQt.exe"
if exist "deploy\minimal\bin\JitsiMeetQt.exe" set FILES_TO_SIGN=%FILES_TO_SIGN% "deploy\minimal\bin\JitsiMeetQt.exe"
if exist "deploy\debug\bin\JitsiMeetQt.exe" set FILES_TO_SIGN=%FILES_TO_SIGN% "deploy\debug\bin\JitsiMeetQt.exe"

REM 查找安装程序
for %%f in ("build\JitsiMeetQt-*.exe") do (
    if exist "%%f" set FILES_TO_SIGN=!FILES_TO_SIGN! "%%f"
)
for %%f in ("packages\*Setup*.exe") do (
    if exist "%%f" set FILES_TO_SIGN=!FILES_TO_SIGN! "%%f"
)

if "%FILES_TO_SIGN%"=="" (
    echo Warning: No files found to sign
    echo Please build the project first
    pause
    exit /b 1
)

echo Files to sign:%FILES_TO_SIGN%

REM 根据证书类型设置签名参数
if "%CERT_TYPE%"=="file" (
    REM 文件证书签名
    if not exist "%CERT_FILE%" (
        echo Warning: Certificate file not found: %CERT_FILE%
        echo.
        echo To enable code signing:
        echo 1. Obtain a code signing certificate from a trusted CA
        echo 2. Export it as a .p12/.pfx file and place it as %CERT_FILE%
        echo 3. Create %CERT_PASSWORD_FILE% with the certificate password
        echo.
        echo Skipping code signing...
        pause
        goto :eof
    )
    
    if not exist "%CERT_PASSWORD_FILE%" (
        echo Error: Certificate password file not found: %CERT_PASSWORD_FILE%
        echo Please create this file with your certificate password
        pause
        exit /b 1
    )
    
    set /p CERT_PASSWORD=<%CERT_PASSWORD_FILE%
    set SIGN_PARAMS=/f "%CERT_FILE%" /p "!CERT_PASSWORD!"
    
) else if "%CERT_TYPE%"=="store" (
    REM 证书存储签名
    if "%CERT_SUBJECT_NAME%"=="" (
        echo Error: Certificate subject name not specified
        echo Use --cert-store "Your Certificate Subject Name"
        pause
        exit /b 1
    )
    
    set SIGN_PARAMS=/s "%CERT_STORE_NAME%" /n "%CERT_SUBJECT_NAME%"
    
) else (
    echo Error: Invalid certificate type: %CERT_TYPE%
    pause
    exit /b 1
)

echo.
echo ========================================
echo Signing files...
echo ========================================

set SIGN_SUCCESS=0
set SIGN_FAILED=0

for %%f in (%FILES_TO_SIGN%) do (
    if exist %%f (
        echo.
        echo Signing: %%f
        
        REM 第一次签名 - SHA256
        %SIGNTOOL% sign %SIGN_PARAMS% /fd %HASH_ALGORITHM% /tr "%TIMESTAMP_RFC3161%" /td %HASH_ALGORITHM% /v %%f
        
        if errorlevel 1 (
            echo Error: Failed to sign %%f
            set /a SIGN_FAILED+=1
        ) else (
            echo Successfully signed: %%f
            set /a SIGN_SUCCESS+=1
        )
    ) else (
        echo Warning: File not found: %%f
    )
)

echo.
echo ========================================
echo Verifying signatures...
echo ========================================

set VERIFY_SUCCESS=0
set VERIFY_FAILED=0

for %%f in (%FILES_TO_SIGN%) do (
    if exist %%f (
        echo.
        echo Verifying: %%f
        %SIGNTOOL% verify /pa /v %%f
        
        if errorlevel 1 (
            echo Verification failed: %%f
            set /a VERIFY_FAILED+=1
        ) else (
            echo Verification successful: %%f
            set /a VERIFY_SUCCESS+=1
        )
    )
)

echo.
echo ========================================
echo Code Signing Summary
echo ========================================

echo Files signed successfully: %SIGN_SUCCESS%
echo Files failed to sign: %SIGN_FAILED%
echo Files verified successfully: %VERIFY_SUCCESS%
echo Files failed verification: %VERIFY_FAILED%

if %SIGN_FAILED% gtr 0 (
    echo.
    echo Warning: Some files failed to sign
    echo Please check the certificate configuration and try again
)

if %VERIFY_FAILED% gtr 0 (
    echo.
    echo Warning: Some signatures failed verification
    echo The signed files may not be trusted by Windows
)

if %SIGN_SUCCESS% gtr 0 (
    if %SIGN_FAILED%==0 (
        if %VERIFY_FAILED%==0 (
            echo.
            echo All files signed and verified successfully!
            echo The application is ready for distribution.
        )
    )
)

REM 显示证书信息
echo.
echo ========================================
echo Certificate Information
echo ========================================

for %%f in (%FILES_TO_SIGN%) do (
    if exist %%f (
        echo.
        echo Certificate details for: %%f
        %SIGNTOOL% verify /v %%f | findstr /C:"Issued to:" /C:"Issued by:" /C:"Expires:" /C:"SHA1 hash:"
        goto :cert_info_done
    )
)
:cert_info_done

echo.
echo Code signing completed on: %DATE% %TIME%

pause