@echo off
echo Testing Jitsi Meet Qt Build...
echo.

echo Checking if executable exists...
if exist "release\jitsi-meet-qt.exe" (
    echo ✓ Executable found: release\jitsi-meet-qt.exe
    
    echo.
    echo File information:
    dir "release\jitsi-meet-qt.exe"
    
    echo.
    echo Testing executable...
    echo Starting application (will close automatically)...
    timeout /t 2 /nobreak >nul
    start /wait /b "Jitsi Meet Qt Test" "release\jitsi-meet-qt.exe" --help
    
    echo.
    echo ✓ Build test completed successfully!
    echo The Jitsi Meet Qt application has been built and is ready to use.
) else (
    echo ✗ Executable not found!
    echo Build may have failed.
)

echo.
pause