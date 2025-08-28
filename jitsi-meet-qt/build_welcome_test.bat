@echo off
echo Building WelcomeWindow test...

REM Set Qt paths - adjust these paths according to your Qt installation
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64

REM Add Qt and MinGW to PATH
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_test mkdir build_test
cd build_test

REM Generate Makefile using qmake
%QT_DIR%\bin\qmake.exe -project -o test_welcome.pro ../test_welcome_window.cpp
echo QT += core widgets network websockets multimedia multimediawidgets >> test_welcome.pro
echo CONFIG += c++17 >> test_welcome.pro
echo INCLUDEPATH += ../include >> test_welcome.pro
echo SOURCES += ../src/WelcomeWindow.cpp ../src/NavigationBar.cpp ../src/RecentListWidget.cpp ../src/models/RecentItem.cpp ../src/ConfigurationManager.cpp ../src/models/ApplicationSettings.cpp >> test_welcome.pro

%QT_DIR%\bin\qmake.exe test_welcome.pro

REM Build the project
mingw32-make.exe

echo Build complete!
pause