@echo off
echo Building simple WindowManager test...

REM 设置Qt环境变量
set QT_DIR=C:\Qt\6.8.3\mingw_64
if exist "%QT_DIR%" (
    set PATH=%QT_DIR%\bin;%PATH%
    echo Using Qt from: %QT_DIR%
) else (
    echo Qt directory not found at %QT_DIR%
    echo Please install Qt 6.8.3 or update the path in this script
    pause
    exit /b 1
)

REM 编译简单测试
echo Compiling simple test...
g++ -std=c++17 -I./include -I"%QT_DIR%\include" -I"%QT_DIR%\include\QtCore" -I"%QT_DIR%\include\QtWidgets" -I"%QT_DIR%\include\QtGui" -L"%QT_DIR%\lib" -lQt6Core -lQt6Widgets -lQt6Gui -o test_window_manager_simple.exe test_window_manager_simple.cpp

if exist test_window_manager_simple.exe (
    echo Simple test compiled successfully!
    echo Running simple test...
    test_window_manager_simple.exe
    echo.
    echo Test completed.
) else (
    echo Simple test compilation failed!
    echo Trying alternative compilation...
    
    REM 尝试使用qmake方式
    echo QT += core widgets > simple_test.pro
    echo CONFIG += console >> simple_test.pro
    echo TARGET = test_window_manager_simple >> simple_test.pro
    echo SOURCES += test_window_manager_simple.cpp >> simple_test.pro
    echo INCLUDEPATH += include >> simple_test.pro
    
    qmake simple_test.pro
    if exist Makefile (
        mingw32-make
        if exist test_window_manager_simple.exe (
            echo Alternative compilation successful!
            test_window_manager_simple.exe
        ) else (
            echo Alternative compilation also failed!
        )
    ) else (
        echo qmake failed to generate Makefile
    )
)

pause