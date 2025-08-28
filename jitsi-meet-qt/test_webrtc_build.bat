@echo off
echo Building WebRTC Engine Test...

REM Create build directory
if not exist "build_webrtc_test" mkdir build_webrtc_test
cd build_webrtc_test

REM Configure with CMake (assuming Qt is in PATH)
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug

REM Build the test
mingw32-make -j4

REM Check if build was successful
if exist "JitsiMeetQt.exe" (
    echo Build successful!
    echo.
    echo To run the WebRTC test, compile the test file separately:
    echo g++ -I../include -I%QTDIR%/include -L%QTDIR%/lib ../test_webrtc_simple.cpp ../src/WebRTCEngine.cpp -lQt6Core -lQt6Widgets -lQt6Multimedia -lQt6MultimediaWidgets -lQt6Network -lQt6WebSockets -o webrtc_test.exe
) else (
    echo Build failed!
    exit /b 1
)

cd ..