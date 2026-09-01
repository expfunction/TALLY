@echo off
setlocal

set SCRIPT_DIR=%~dp0
if not exist "%SCRIPT_DIR%BIN" mkdir "%SCRIPT_DIR%BIN"

:: Check if INCLUDE is set, otherwise auto-initialize Visual Studio x64 environment
if "%INCLUDE%"=="" (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" (
                call "%%i\VC\Auxiliary\Build\vcvars64.bat"
            )
        )
    )
)

echo Building COUNTED [TRUST NO ONE] Release Build (Win64 MSVC)...

:: Optimized release flags
set "CFLAGS=/TC /MD /std:c17 /W3 /wd4996 /wd4116 /GS- /arch:AVX2 /nologo /O2 /I"%SCRIPT_DIR%include" /DPLATFORM_WIN64 /DWIN32_LEAN_AND_MEAN /DGLEW_STATIC /D_USE_MATH_DEFINES /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS"
set "LFLAGS=/NOLOGO /SUBSYSTEM:CONSOLE /MACHINE:X64 /NODEFAULTLIB:libcmt.lib"

echo Compiling TALLY.C in Release mode...
cl.exe %CFLAGS% /Fo"%SCRIPT_DIR%BIN\TALLY_RELEASE.obj" /c "%SCRIPT_DIR%TALLY.C"
if errorlevel 1 (
    echo Compilation failed.
    goto :EOF
)

echo Linking COUNTED_WIN64.exe ...
link.exe %LFLAGS% /OUT:"%SCRIPT_DIR%BIN\COUNTED_WIN64.exe" "%SCRIPT_DIR%BIN\TALLY_RELEASE.obj" "%SCRIPT_DIR%lib\libcvga.lib"
if errorlevel 1 (
    echo Link failed.
    goto :EOF
)

echo.
echo =======================================================
echo Full Release Build Successful!
echo Output: BIN\COUNTED_WIN64.exe
echo =======================================================

