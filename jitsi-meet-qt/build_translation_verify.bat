@echo off
echo Building Translation Manager Verification Test...

REM Set Qt paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_translation_verify mkdir build_translation_verify
cd build_translation_verify

REM Generate Makefile
qmake -project
echo "QT += core widgets network multimedia multimediawidgets" >> *.pro
echo "CONFIG += c++17" >> *.pro
echo "TARGET = verify_translation_manager" >> *.pro
echo "SOURCES += ../verify_translation_manager.cpp ../src/TranslationManager.cpp" >> *.pro
echo "HEADERS += ../include/TranslationManager.h" >> *.pro
echo "INCLUDEPATH += ../include" >> *.pro

REM Build
qmake
mingw32-make

if exist verify_translation_manager.exe (
    echo Build successful!
    echo Running verification test...
    verify_translation_manager.exe
) else (
    echo Build failed!
)

cd ..
pause