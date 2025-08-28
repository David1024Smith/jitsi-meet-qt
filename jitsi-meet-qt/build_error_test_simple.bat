@echo off
echo Building Error Handling Integration Test...

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Clean previous build
if exist test_error_integration.exe del test_error_integration.exe
if exist Makefile del Makefile

REM Generate Makefile and build
qmake test_error_integration.pro
mingw32-make

if exist test_error_integration.exe (
    echo.
    echo Build successful! Running error handling integration test...
    echo.
    test_error_integration.exe
    echo.
    echo Error handling integration test completed.
) else (
    echo.
    echo Build failed! Please check the error messages above.
)

pause