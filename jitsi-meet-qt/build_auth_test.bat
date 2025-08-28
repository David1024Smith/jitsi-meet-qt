@echo off
echo Building AuthenticationManager test...

REM Set Qt environment
set QT_DIR=C:\Qt\6.8.3\mingw_64
set PATH=%QT_DIR%\bin;%PATH%

REM Create build directory
if not exist "build_auth_test" mkdir build_auth_test
cd build_auth_test

REM Generate Makefile
qmake -project
echo "QT += core network testlib" >> *.pro
echo "CONFIG += testcase" >> *.pro
echo "TARGET = test_authentication_manager" >> *.pro
echo "SOURCES += ../test_authentication_manager.cpp ../src/AuthenticationManager.cpp" >> *.pro
echo "HEADERS += ../include/AuthenticationManager.h" >> *.pro
echo "INCLUDEPATH += ../include" >> *.pro

REM Build
qmake
mingw32-make

REM Run test
if exist "test_authentication_manager.exe" (
    echo Running AuthenticationManager tests...
    test_authentication_manager.exe
) else (
    echo Build failed - executable not found
)

cd ..
echo AuthenticationManager test completed.