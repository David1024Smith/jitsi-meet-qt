@echo off
echo Building Error Handling Integration Test...

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_error_integration mkdir build_error_integration
cd build_error_integration

REM Generate Makefile
qmake -project -o test_error_integration.pro
echo CONFIG += console >> test_error_integration.pro
echo QT += core widgets network websockets multimedia multimediawidgets >> test_error_integration.pro
echo TARGET = test_error_integration >> test_error_integration.pro
echo INCLUDEPATH += ../include >> test_error_integration.pro

REM Add source files to project
echo SOURCES += ../test_error_handling_integration.cpp >> test_error_integration.pro
echo SOURCES += ../src/JitsiError.cpp >> test_error_integration.pro
echo SOURCES += ../src/ErrorRecoveryManager.cpp >> test_error_integration.pro
echo SOURCES += ../src/ErrorDialog.cpp >> test_error_integration.pro
echo SOURCES += ../src/ErrorUtils.cpp >> test_error_integration.pro

REM Add header files
echo HEADERS += ../include/JitsiError.h >> test_error_integration.pro
echo HEADERS += ../include/ErrorRecoveryManager.h >> test_error_integration.pro
echo HEADERS += ../include/ErrorDialog.h >> test_error_integration.pro
echo HEADERS += ../include/ErrorUtils.h >> test_error_integration.pro
echo HEADERS += ../include/JitsiConstants.h >> test_error_integration.pro

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

cd ..
pause