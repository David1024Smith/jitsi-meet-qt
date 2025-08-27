@echo off
echo Building translation files...

set QT_DIR=C:\Qt\5.15.2\mingw81_64
set LRELEASE=%QT_DIR%\bin\lrelease.exe

if not exist "%LRELEASE%" (
    echo Error: lrelease.exe not found at %LRELEASE%
    echo Please update QT