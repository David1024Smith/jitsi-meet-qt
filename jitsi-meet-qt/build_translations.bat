@echo off
echo Building translation files...

set QT_DIR=C:\Qt\5.15.2\mingw81_64
set LRELEASE=%QT_DIR%\bin\lrelease.exe
set LUPDATE=%QT_DIR%\bin\lupdate.exe

if not exist "%LRELEASE%" (
    echo Error: lrelease.exe not found at %LRELEASE%
    echo Please update QT_DIR variable to point to your Qt installation
    pause
    exit /b 1
)

if not exist "%LUPDATE%" (
    echo Error: lupdate.exe not found at %LUPDATE%
    echo Please update QT_DIR variable to point to your Qt installation
    pause
    exit /b 1
)

echo Updating translation source files...
"%LUPDATE%" -recursive src include -ts translations/jitsi_en.ts translations/jitsi_zh_CN.ts translations/jitsi_ja.ts translations/jitsi_ko.ts translations/jitsi_fr.ts translations/jitsi_de.ts translations/jitsi_es.ts translations/jitsi_ru.ts

echo Compiling translation files...
for %%f in (translations\*.ts) do (
    echo Compiling %%f...
    "%LRELEASE%" "%%f"
    if errorlevel 1 (
        echo Error compiling %%f
        pause
        exit /b 1
    )
)

echo Translation files built successfully!
echo Compiled .qm files are ready for use.
pause