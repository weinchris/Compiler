#!/bin/bash

echo Step 1  - Build Bison parser
bison -d compiler.y -b generated\compiler || exit 1;

echo Step 2  - Build flex scanner
flex -ogenerated\lex.yy.c compiler.l || exit 1;

echo Step 3  - Compile parser
gcc -g -c generated\compiler.tab.c -o bin\compiler_y.o || exit 1;

echo Step 4  - Compile scanner
gcc -g -c generated\lex.yy.c -o bin\lex.yy.o || exit 1;

echo Step 5a - Compile compiler.c
gcc -g -c compiler.c -o bin\compiler.o || exit 1;

echo Step 5b - Compile symboltable.c
gcc -g -c symboltable.c -o bin\symboltable.o || exit 1;

echo Step 5c - Compile generator.c
gcc -g -c generator.c -o bin\generator.o || exit 1;

echo Step 5d - Compile optimizer.c
gcc -g -c optimizer.c -o bin\optimizer.o || exit 1;

echo Step 5e - Compile interpreter.c
gcc -g -c interpreter.c -o bin\interpreter.o || exit 1;

echo Step 6  - Link compile result
gcc -g -o bin\compiler.exe bin\compiler.o bin\symboltable.o bin\generator.o bin\optimizer.o bin\interpreter.o bin\compiler_y.o bin\lex.yy.o -lm -lfl || exit 1;
