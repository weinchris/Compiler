/**
 * @file compiler.h
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This defines all general functions for the compiler.
 */

#include "symboltable.h"
#include <stdio.h>

#ifndef COMPILER_H_
#define COMPILER_H_

/**
 * Reference to standard C function to prevent compiler warnings.
 */
extern int yyparse();

/**
 * Reference to standard C function to prevent compiler warnings.
 */
extern char *strdup(const char *s);

/**
 * This function is called by the parser if an error has been detected while
 * parsing the input data (e.g. syntax error).
 * @param str The error message to be printed.
 */
void yyerror(const char* str);

char* helperVariableCounter();

dataType getType(symbolTableEntry *firstEntry, symbolTableEntry *secondEntry);

int hasTypeConflict(dataType firstEntry, dataType secondEntry);

#endif /*COMPILER_H_*/
