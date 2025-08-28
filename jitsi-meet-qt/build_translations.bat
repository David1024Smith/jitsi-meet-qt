@echo off
echo Building Translation Files...

REM Set Qt paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create translations directory if it doesn't exist
if not exist translations mkdir translations

echo Compiling translation files...

REM Compile .ts files to .qm files
lrelease translations\jitsimeet_en.ts -qm translations\jitsimeet_en.qm
lrelease translations\jitsimeet_zh_CN.ts -qm translations\jitsimeet_zh_CN.qm
lrelease translations\jitsimeet_es.ts -qm translations\jitsimeet_es.qm

echo Translation files compiled successfully!

REM List compiled files
echo Compiled translation files:
dir translations\*.qm

pause