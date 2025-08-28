@echo off
echo Building Translation Manager Test...

REM Set Qt paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_translation_test mkdir build_translation_test
cd build_translation_test

REM Generate Makefile
qmake -project
echo "QT += core widgets network multimedia multimediawidgets" >> *.pro
echo "CONFIG += c++17" >> *.pro
echo "TARGET = test_translation_simple" >> *.pro
echo "SOURCES += ../test_translation_simple.cpp ../src/TranslationManager.cpp" >> *.pro
echo "HEADERS += ../include/TranslationManager.h" >> *.pro
echo "INCLUDEPATH += ../include" >> *.pro

REM Build
qmake
mingw32-make

if exist test_translation_simple.exe (
    echo Build successful!
    echo Running test...
    test_translation_simple.exe
) else (
    echo Build failed!
)

cd ..
pause