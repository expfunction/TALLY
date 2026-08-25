@echo off
if "%1"=="" goto usage

echo Jacking in (DJGPP)...
echo Compiling %1.C ...

gcc -O2 -Iinclude -x c -c %1.C
if errorlevel 1 goto fail_comp

echo Linking %1.exe ...
gcc -o %1.exe %1.o lib\libcvga.a -lm
if errorlevel 1 goto fail_link

if not exist BIN\NUL mkdir BIN
copy %1.exe BIN\ > NUL
del %1.exe
del %1.o

echo Success! Output: BIN\%1.exe
goto end

:fail_comp
echo Compilation failed.
goto end

:fail_link
echo Link failed.
goto end

:usage
echo [!] ERROR: No source file specified.
echo Usage: JACKINDOS.bat filename (without extension)
echo Example: JACKINDOS MAIN

:end
