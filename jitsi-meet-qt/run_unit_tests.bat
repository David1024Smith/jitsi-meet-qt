@echo off
echo ========================================
echo Running JitsiMeetQt Unit Tests
echo ========================================

REM Set Qt environment
set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;%PATH%

REM Check if tests are built
if not exist "build_unit_tests\debug\test_unit_all.exe" (
    echo Unit tests not found. Building first...
    call build_unit_tests.bat
    if %ERRORLEVEL% neq 0 (
        echo Build failed! Cannot run tests.
        pause
        exit /b 1
    )
)

echo.
echo Running individual unit test suites...
echo.

REM Create test results directory
if not exist "test_results" mkdir test_results

REM Run XMPPClient tests
echo 1. Running XMPPClient unit tests...
cd build_unit_tests\debug
if exist "test_unit_xmpp_client.exe" (
    test_unit_xmpp_client.exe > ..\..\test_results\xmpp_client_results.txt 2>&1
    echo    XMPPClient tests completed
) else (
    echo    XMPPClient test executable not found
)

REM Run WebRTCEngine tests
echo 2. Running WebRTCEngine unit tests...
if exist "test_unit_webrtc_engine.exe" (
    test_unit_webrtc_engine.exe > ..\..\test_results\webrtc_engine_results.txt 2>&1
    echo    WebRTCEngine tests completed
) else (
    echo    WebRTCEngine test executable not found
)

REM Run ConfigurationManager tests
echo 3. Running ConfigurationManager unit tests...
if exist "test_unit_configuration_manager.exe" (
    test_unit_configuration_manager.exe > ..\..\test_results\configuration_manager_results.txt 2>&1
    echo    ConfigurationManager tests completed
) else (
    echo    ConfigurationManager test executable not found
)

REM Run ChatManager tests
echo 4. Running ChatManager unit tests...
if exist "test_unit_chat_manager.exe" (
    test_unit_chat_manager.exe > ..\..\test_results\chat_manager_results.txt 2>&1
    echo    ChatManager tests completed
) else (
    echo    ChatManager test executable not found
)

REM Run MediaManager tests
echo 5. Running MediaManager unit tests...
if exist "test_unit_media_manager.exe" (
    test_unit_media_manager.exe > ..\..\test_results\media_manager_results.txt 2>&1
    echo    MediaManager tests completed
) else (
    echo    MediaManager test executable not found
)

REM Run comprehensive test suite
echo.
echo Running comprehensive test suite...
if exist "test_unit_all.exe" (
    test_unit_all.exe > ..\..\test_results\all_tests_results.txt 2>&1
    echo Comprehensive tests completed
) else (
    echo Comprehensive test executable not found
)

cd ..\..

echo.
echo ========================================
echo Unit Tests Execution Completed!
echo ========================================
echo.
echo Test results saved to: test_results\
echo.
echo Individual test results:
if exist "test_results\xmpp_client_results.txt" echo   - XMPPClient: test_results\xmpp_client_results.txt
if exist "test_results\webrtc_engine_results.txt" echo   - WebRTCEngine: test_results\webrtc_engine_results.txt
if exist "test_results\configuration_manager_results.txt" echo   - ConfigurationManager: test_results\configuration_manager_results.txt
if exist "test_results\chat_manager_results.txt" echo   - ChatManager: test_results\chat_manager_results.txt
if exist "test_results\media_manager_results.txt" echo   - MediaManager: test_results\media_manager_results.txt
if exist "test_results\all_tests_results.txt" echo   - Comprehensive: test_results\all_tests_results.txt
echo.

pause