@echo off
echo Building WindowManager test...

REM 设置Qt环境
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM 创建构建目录
if not exist build_window_test mkdir build_window_test
cd build_window_test

REM 生成Makefile
qmake -project
echo "QT += core widgets testlib" >> *.pro
echo "CONFIG += testcase" >> *.pro
echo "TARGET = test_window_manager" >> *.pro
echo "SOURCES += ../test_window_manager.cpp" >> *.pro
echo "SOURCES += ../src/WindowManager.cpp" >> *.pro
echo "SOURCES += ../src/WindowStateManager.cpp" >> *.pro
echo "SOURCES += ../src/ConfigurationManager.cpp" >> *.pro
echo "SOURCES += ../src/TranslationManager.cpp" >> *.pro
echo "SOURCES += ../src/WelcomeWindow.cpp" >> *.pro
echo "SOURCES += ../src/ConferenceWindow.cpp" >> *.pro
echo "SOURCES += ../src/SettingsDialog.cpp" >> *.pro
echo "HEADERS += ../include/WindowManager.h" >> *.pro
echo "HEADERS += ../include/WindowStateManager.h" >> *.pro
echo "HEADERS += ../include/ConfigurationManager.h" >> *.pro
echo "HEADERS += ../include/TranslationManager.h" >> *.pro
echo "HEADERS += ../include/WelcomeWindow.h" >> *.pro
echo "HEADERS += ../include/ConferenceWindow.h" >> *.pro
echo "HEADERS += ../include/SettingsDialog.h" >> *.pro
echo "INCLUDEPATH += ../include" >> *.pro

qmake
mingw32-make

if exist test_window_manager.exe (
    echo Test compiled successfully!
    echo Running WindowManager test...
    test_window_manager.exe
) else (
    echo Test compilation failed!
)

cd ..