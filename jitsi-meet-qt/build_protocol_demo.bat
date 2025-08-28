@echo off
echo Building Protocol Handler Demo...

REM 创建构建目录
if not exist build mkdir build
cd build

REM 配置CMake
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM 构建协议处理器演示
echo Building protocol handler demo...
g++ -std=c++17 -I../include -I../src ^
    -I%QTDIR%/include -I%QTDIR%/include/QtCore -I%QTDIR%/include/QtWidgets -I%QTDIR%/include/QtNetwork ^
    -L%QTDIR%/lib ^
    ../examples/protocol_handler_demo.cpp ^
    ../src/ProtocolHandler.cpp ^
    -lQt5Core -lQt5Widgets -lQt5Network ^
    -o protocol_handler_demo.exe

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Demo built successfully!
echo Run protocol_handler_demo.exe to test the protocol handler
pause