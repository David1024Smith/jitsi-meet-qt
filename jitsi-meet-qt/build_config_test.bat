@echo off
echo Building ConfigurationManager test...

REM 设置编译器路径（假设使用MinGW）
set PATH=C:\Qt\Tools\mingw1120_64\bin;%PATH%
set PATH=C:\Qt\6.8.3\mingw_64\bin;%PATH%

REM 创建构建目录
if not exist build_config_test mkdir build_config_test
cd build_config_test

REM 生成 Makefile
qmake -project -o config_test.pro ..
echo CONFIG += console >> config_test.pro
echo QT += core >> config_test.pro
echo TARGET = config_test >> config_test.pro
echo SOURCES += ../test_configuration_manager.cpp >> config_test.pro
echo SOURCES += ../src/ConfigurationManager.cpp >> config_test.pro
echo SOURCES += ../src/models/ApplicationSettings.cpp >> config_test.pro
echo SOURCES += ../src/WindowStateManager.cpp >> config_test.pro
echo HEADERS += ../include/ConfigurationManager.h >> config_test.pro
echo HEADERS += ../include/models/ApplicationSettings.h >> config_test.pro
echo HEADERS += ../include/WindowStateManager.h >> config_test.pro
echo HEADERS += ../include/JitsiConstants.h >> config_test.pro
echo INCLUDEPATH += ../include >> config_test.pro
echo INCLUDEPATH += ../include/models >> config_test.pro

REM 编译
qmake config_test.pro
mingw32-make

REM 运行测试
if exist config_test.exe (
    echo.
    echo Running ConfigurationManager tests...
    echo =====================================
    config_test.exe
    echo =====================================
    echo Test completed.
) else (
    echo Build failed - executable not found
)

cd ..
pause