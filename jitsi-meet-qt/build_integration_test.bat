@echo off
echo Building Jitsi Meet Qt Integration Test...

REM Set Qt path (adjust as needed)
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%

REM Create build directory
if not exist build_integration mkdir build_integration
cd build_integration

REM Generate MOC file
%QT_DIR%\bin\moc.exe ..\test_integration_simple.cpp -o test_integration_simple.moc

REM Compile the test
g++ -std=c++17 -I%QT_DIR%\include -I%QT_DIR%\include\QtCore -I%QT_DIR%\include\QtWidgets -I%QT_DIR%\include\QtGui ^
    ..\test_integration_simple.cpp ^
    -L%QT_DIR%\lib ^
    -lQt6Core -lQt6Widgets -lQt6Gui ^
    -o test_integration_simple.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running integration test...
    test_integration_simple.exe
) else (
    echo Build failed!
)

cd ..