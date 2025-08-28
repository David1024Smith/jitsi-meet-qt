@echo off
echo Building Resource Verification Tool...

REM Set Qt and MinGW paths
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

REM Create build directory
if not exist build_verify_resources mkdir build_verify_resources
cd build_verify_resources

REM Create simple CMakeLists.txt for verification tool
echo cmake_minimum_required(VERSION 3.20) > CMakeLists.txt
echo project(VerifyResources) >> CMakeLists.txt
echo set(CMAKE_CXX_STANDARD 17) >> CMakeLists.txt
echo set(CMAKE_AUTOMOC ON) >> CMakeLists.txt
echo set(CMAKE_AUTORCC ON) >> CMakeLists.txt
echo find_package(Qt6 REQUIRED COMPONENTS Core Widgets) >> CMakeLists.txt
echo add_executable(verify_resources ../verify_resources.cpp ../resources/resources.qrc) >> CMakeLists.txt
echo target_link_libraries(verify_resources Qt6::Core Qt6::Widgets) >> CMakeLists.txt

REM Configure with CMake
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=%QT_DIR% .

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
mingw32-make

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Running resource verification...

REM Run the verification
verify_resources.exe

pause