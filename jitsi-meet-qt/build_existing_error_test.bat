@echo off
echo Building Existing Error Test...

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Clean previous build
if exist test_error_existing.exe del test_error_existing.exe

REM Compile directly
g++ -std=c++17 -I include -I %QT_DIR%\include -I %QT_DIR%\include\QtCore -I %QT_DIR%\include\QtNetwork -L %QT_DIR%\lib -lQt6Core -lQt6Network -o test_error_existing.exe test_error_simple.cpp src\JitsiError.cpp

if exist test_error_existing.exe (
    echo.
    echo Build successful! Running existing error test...
    echo.
    test_error_existing.exe
    echo.
    echo Existing error test completed.
) else (
    echo.
    echo Build failed! Please check the error messages above.
)

pause