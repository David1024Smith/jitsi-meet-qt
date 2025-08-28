@echo off
echo Building WindowManager verification...

REM 设置Qt环境
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM 创建构建目录
if not exist build_window_verify mkdir build_window_verify
cd build_window_verify

REM 生成Makefile
qmake -project
echo "QT += core widgets" >> *.pro
echo "CONFIG += console" >> *.pro
echo "TARGET = verify_window_manager" >> *.pro
echo "SOURCES += ../verify_window_manager.cpp" >> *.pro
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

if exist verify_window_manager.exe (
    echo Verification compiled successfully!
    echo Running WindowManager verification...
    verify_window_manager.exe
) else (
    echo Verification compilation failed!
)

cd ..