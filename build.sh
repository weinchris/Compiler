#!/bin/bash

echo "Step 1 Bison Parser"
bison -d compiler.y -b generated/compiler || { exit 1; }

echo "Step 2 Flex Scanner"
flex -o generated/lex.yy.c compiler.l || { exit 1; }

echo "Step 3 Compile Parser"
gcc -g -c generated/compiler.tab.c -o bin/compiler_y.o || { exit 1; }

echo "Step 4 Compile Scanner"
gcc -g -c generated/lex.yy.c -o bin/lex.yy.o || { exit 1; }

echo "Step 5a Compiler.o"
gcc -g -c compiler.c -o bin/compiler.o || { exit 1; }

echo "Step 5b Symboltabelle.o"
gcc -g -c symboltable.c -o bin/symboltable.o || { exit 1; }

echo "Step 5c Generator.o"
gcc -g -c generator.c -o bin/generator.o || { exit 1; }

echo "Step 5d Optimizer.o"
gcc -g -c optimizer.c -o bin/optimizer.o || { exit 1; }

echo "Step 5e Interpeter.o"
gcc -g -c interpreter.c -o bin/interpreter.o || { exit 1; }

echo "Step 6 Link result"
gcc -g -o bin/compiler bin/compiler.o bin/symboltable.o bin/generator.o bin/optimizer.o bin/interpreter.o bin/compiler_y.o bin/lex.yy.o -lm -ll || { exit 1; }
