@echo off
echo Building Translation Unit Test...

REM Set Qt paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Compile directly with g++
echo Compiling with g++...
g++ -std=c++17 -I%QT_DIR%\include -I%QT_DIR%\include\QtCore -I%QT_DIR%\include\QtWidgets ^
    -L%QT_DIR%\lib -lQt6Core -lQt6Widgets ^
    test_translation_unit.cpp -o test_translation_unit.exe

if exist test_translation_unit.exe (
    echo Build successful!
    echo Running unit test...
    test_translation_unit.exe
) else (
    echo Build failed! Trying alternative approach...
    
    REM Try with qmake
    echo QT += core > test_unit.pro
    echo CONFIG += console >> test_unit.pro
    echo CONFIG -= app_bundle >> test_unit.pro
    echo TARGET = test_translation_unit >> test_unit.pro
    echo SOURCES += test_translation_unit.cpp >> test_unit.pro
    
    qmake test_unit.pro
    mingw32-make
    
    if exist test_translation_unit.exe (
        echo Build successful with qmake!
        echo Running unit test...
        test_translation_unit.exe
    ) else (
        echo Build failed with both methods!
    )
)

pause