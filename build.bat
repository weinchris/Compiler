@echo off

setlocal
set ROOT_DIR=%CD%

set BISON_DIR=%ROOT_DIR%\Tools\bison
set FLEX_DIR=%ROOT_DIR%\Tools\flex
set GCC_DIR=%ROOT_DIR%\Tools\MinGW

set PATH=%BISON_DIR%\bin;%FLEX_DIR%\bin;%GCC_DIR%\bin;%PATH%

echo Step 1  - Build Bison parser
bison -d compiler.y -b generated\compiler
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 2  - Build flex scanner
flex -ogenerated\lex.yy.c compiler.l
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 3  - Compile parser
gcc -g -c generated\compiler.tab.c -o bin\compiler_y.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 4  - Compile scanner
gcc -g -c generated\lex.yy.c -o bin\lex.yy.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 5a - Compile compiler.c
gcc -g -c compiler.c -o bin\compiler.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 5b - Compile symboltable.c
gcc -g -c symboltable.c -o bin\symboltable.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 5c - Compile generator.c
gcc -g -c generator.c -o bin\generator.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 5d - Compile optimizer.c
gcc -g -c optimizer.c -o bin\optimizer.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 5e - Compile interpreter.c
gcc -g -c interpreter.c -o bin\interpreter.o
IF %ERRORLEVEL% GEQ 1 goto :error

echo Step 6  - Link compile result
gcc -g -o bin\compiler.exe bin\compiler.o bin\symboltable.o bin\generator.o bin\optimizer.o bin\interpreter.o bin\compiler_y.o bin\lex.yy.o -lm -lfl
IF %ERRORLEVEL% GEQ 1 goto :error

goto :eof

:error
echo -----
echo Failed to compile
echo -----
