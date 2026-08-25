@echo off
setlocal

if "%~1"=="" (
    echo Usage: JACKINWIN64.BAT [script.c]
    echo Compiles and links an external script with the CyberVGA engine using MSVC.
    echo NOTE: Must be run from an x64 Native Tools Command Prompt.
    goto :EOF
)

set SRC_FILE=%~1
set BASENAME=%~n1
set SCRIPT_DIR=%~dp0

if not exist "%SCRIPT_DIR%BIN" mkdir "%SCRIPT_DIR%BIN"

if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo [!] ERROR: Incorrect build environment!
    echo Please run this script from the "x64 Native Tools Command Prompt for VS 2022".
    echo The standard Developer Command Prompt defaults to x86, which causes linker conflicts.
    goto :EOF
)

echo Jacking in (MSVC)...
echo Compiling %SRC_FILE% ...

:: Base compiler and linker flags for CyberVGA
set "CFLAGS=/TC /MD /std:c17 /W3 /wd4996 /wd4116 /GS- /arch:AVX2 /nologo /Od /Zi /RTC1 /I"%SCRIPT_DIR%include" /DPLATFORM_WIN64 /DWIN32_LEAN_AND_MEAN /DGLEW_STATIC /D_USE_MATH_DEFINES /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS"
set "LFLAGS=/NOLOGO /SUBSYSTEM:CONSOLE /MACHINE:X64 /DEBUG /NODEFAULTLIB:libcmt.lib"
:: Note: Required third-party static libraries are already bundled into libcvga.lib!
:: Note: Windows system libraries are auto-linked via #pragma comment(lib, ...) in CYBER.H

cl.exe %CFLAGS% /Fo"%SCRIPT_DIR%BIN\%BASENAME%_WIN64.obj" /c "%SRC_FILE%"
if errorlevel 1 (
    echo Compilation failed.
    goto :EOF
)

echo Linking %BASENAME%_WIN64.exe ...
link.exe %LFLAGS% /OUT:"%SCRIPT_DIR%BIN\%BASENAME%_WIN64.exe" "%SCRIPT_DIR%BIN\%BASENAME%_WIN64.obj" "%SCRIPT_DIR%lib\libcvga.lib"
if errorlevel 1 (
    echo Link failed.
    goto :EOF
)

echo Success! Output: BIN\%BASENAME%_WIN64.exe
