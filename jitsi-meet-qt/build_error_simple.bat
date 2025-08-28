@echo off
echo Building Simple Error Handling Test...

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Clean previous build
if exist test_error_simple.exe del test_error_simple.exe
if exist Makefile del Makefile

REM Generate Makefile and build
qmake test_error_simple.pro
mingw32-make

if exist test_error_simple.exe (
    echo.
    echo Build successful! Running error handling test...
    echo.
    test_error_simple.exe
    echo.
    echo Error handling test completed.
) else (
    echo.
    echo Build failed! Please check the error messages above.
)

pause