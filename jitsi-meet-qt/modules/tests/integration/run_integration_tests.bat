@echo off
REM Module Integration Test Runner for Windows

echo ========================================
echo Jitsi Meet Qt Module Integration Tests
echo ========================================

REM Set environment variables
set TEST_DIR=%~dp0
set BUILD_DIR=%TEST_DIR%build
set BIN_DIR=%TEST_DIR%bin

REM Create directories if they don't exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

echo.
echo Building integration tests...

REM Check if CMake is available
cmake --version >nul 2>&1
if %errorlevel% equ 0 (
    echo Using CMake build system...
    cd /d "%BUILD_DIR%"
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . --config Debug
    if %errorlevel% neq 0 (
        echo CMake build failed!
        goto :error
    )
    
    echo.
    echo Running integration tests with CMake...
    ctest --output-on-failure -C Debug
    set TEST_RESULT=%errorlevel%
) else (
    REM Fallback to qmake
    echo CMake not found, using qmake...
    cd /d "%TEST_DIR%"
    qmake integration_tests.pro
    if %errorlevel% neq 0 (
        echo qmake failed!
        goto :error
    )
    
    nmake
    if %errorlevel% neq 0 (
        echo Build failed!
        goto :error
    )
    
    echo.
    echo Running integration tests...
    "%BIN_DIR%\ModuleIntegrationTest.exe"
    set TEST_RESULT=%errorlevel%
)

echo.
if %TEST_RESULT% equ 0 (
    echo ========================================
    echo All integration tests PASSED!
    echo ========================================
) else (
    echo ========================================
    echo Some integration tests FAILED!
    echo ========================================
)

REM Generate test report
echo.
echo Generating test report...
if exist "%TEMP%\module_integration_test_report.json" (
    echo Test report available at: %TEMP%\module_integration_test_report.json
    type "%TEMP%\module_integration_test_report.json"
)

goto :end

:error
echo.
echo ========================================
echo Build or test execution failed!
echo ========================================
exit /b 1

:end
exit /b %TEST_RESULT%