@echo off
echo Building ScreenShare Unit Test...

REM Set Qt path - adjust this to your Qt installation
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64

REM Add Qt and MinGW to PATH
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_unit_test mkdir build_unit_test
cd build_unit_test

REM Generate Makefile using qmake
echo Generating Makefile...
qmake -project -o screen_share_unit_test.pro ../test_screen_share_unit.cpp
echo CONFIG += c++17 testcase >> screen_share_unit_test.pro
echo QT += core widgets network websockets multimedia multimediawidgets testlib >> screen_share_unit_test.pro
echo INCLUDEPATH += ../include >> screen_share_unit_test.pro
echo SOURCES += ../src/ScreenShareManager.cpp ../src/WebRTCEngine.cpp >> screen_share_unit_test.pro
echo HEADERS += ../include/ScreenShareManager.h ../include/WebRTCEngine.h >> screen_share_unit_test.pro

qmake screen_share_unit_test.pro

REM Compile
echo Compiling...
mingw32-make

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running unit tests...
    screen_share_unit_test.exe
) else (
    echo Build failed!
)

cd ..
pause