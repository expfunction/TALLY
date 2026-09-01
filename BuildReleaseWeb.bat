@echo off
setlocal

set SCRIPT_DIR=%~dp0

echo ======================================================
echo Packaging COUNTED DOSBox Web (JS-DOS / Itch.io) Release
echo =====================================================
echo.

python "%SCRIPT_DIR%PackageWebDIST.py"
if errorlevel 1 (
    echo [!] Packaging failed.
    pause
    goto :EOF
)

echo.
echo =====================================================
echo Web Release Package Ready!
echo.
echo Upload to Itch.io:
echo   ==^> DIST\COUNTED_WEB_HTML5.zip (or DIST\contdweb.zip)
echo.
echo NOTE for Itch.io:
echo   1. Upload 'COUNTED_WEB_HTML5.zip'
echo   2. Check '[x] This file will be played in the browser'
echo   3. Do NOT upload CONTD.jsdos alone as the browser game.
echo =====================================================
