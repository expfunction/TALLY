@echo off
setlocal

echo Launching COUNTED [TRUST NO ONE] via DOSBox...

:: Try to find dosbox-x or dosbox in standard locations
set "DOSBOX_EXE="

if exist "d:\Program Files\Dosbox-X\dosbox-x.exe" set "DOSBOX_EXE=d:\Program Files\Dosbox-X\dosbox-x.exe"
if exist "%LOCALAPPDATA%\Programs\DOSBox-X\dosbox-x.exe" set "DOSBOX_EXE=%LOCALAPPDATA%\Programs\DOSBox-X\dosbox-x.exe"
if exist "C:\Program Files\DOSBox-X\dosbox-x.exe" set "DOSBOX_EXE=C:\Program Files\DOSBox-X\dosbox-x.exe"
if exist "C:\Program Files (x86)\DOSBox-0.74-3\DOSBox.exe" set "DOSBOX_EXE=C:\Program Files (x86)\DOSBox-0.74-3\DOSBox.exe"

if "%DOSBOX_EXE%"=="" (
    where dosbox-x.exe >nul 2>&1
    if not errorlevel 1 set "DOSBOX_EXE=dosbox-x.exe"
)

if "%DOSBOX_EXE%"=="" (
    where dosbox.exe >nul 2>&1
    if not errorlevel 1 set "DOSBOX_EXE=dosbox.exe"
)

if "%DOSBOX_EXE%"=="" (
    echo [!] ERROR: DOSBox or DOSBox-X was not found in standard paths.
    echo Please install DOSBox-X ^(https://dosbox-x.com/^) or add it to PATH.
    pause
    goto :EOF
)

echo Starting with %DOSBOX_EXE%...
"%DOSBOX_EXE%" -conf dosbox-counted.conf

