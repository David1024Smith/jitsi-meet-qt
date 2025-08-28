@echo off
echo ========================================
echo Building Unit Test Verification Tool
echo ========================================

REM Set Qt environment
set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;%PATH%

echo Building verification tool...
g++ -std=c++17 -I"C:\Qt\6.8.3\mingw_64\include" -I"C:\Qt\6.8.3\mingw_64\include\QtCore" -L"C:\Qt\6.8.3\mingw_64\lib" -lQt6Core verify_unit_tests.cpp -o verify_unit_tests.exe

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo Running unit test verification...
verify_unit_tests.exe

echo.
echo ========================================
echo Unit Test Verification Completed!
echo ========================================

pause