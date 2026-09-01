@echo off
if "%1"=="" goto usage

set OUT_NAME=%2
if "%OUT_NAME%"=="" set OUT_NAME=CONTD

echo Jacking in (DJGPP)...
echo Compiling %1.C ...

gcc -O2 -Iinclude -x c -c %1.C
if errorlevel 1 goto fail_comp

echo Linking %OUT_NAME%.EXE ...
gcc -o %OUT_NAME%.EXE %1.o lib\libcvga.a -lm
if errorlevel 1 goto fail_link

if not exist BIN\NUL mkdir BIN
copy %OUT_NAME%.EXE BIN\ > NUL
del %OUT_NAME%.EXE
del %1.o

echo Success! Output: BIN\%OUT_NAME%.EXE
goto end

:fail_comp
echo Compilation failed.
goto end

:fail_link
echo Link failed.
goto end

:usage
echo [!] ERROR: No source file specified.
echo Usage: JACKINDOS.bat filename [output_name]
echo Example: JACKINDOS TALLY
echo Default output: BIN\CONTD.EXE

:end
