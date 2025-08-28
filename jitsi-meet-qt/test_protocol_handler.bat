@echo off
echo Building and testing Protocol Handler...

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

REM 构建项目
cmake --build . --target test_protocol_handler
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM 运行测试
echo.
echo Running Protocol Handler tests...
tests\test_protocol_handler.exe
if %errorlevel% neq 0 (
    echo Tests failed!
    pause
    exit /b 1
)

echo.
echo All tests passed!
pause