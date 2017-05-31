/**
 * @file compiler.c
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This contains all general function implementations for the compiler.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "symboltable.h"

/**
 * Variable to track current line number.<BR>
 * [defined in file compiler.l]
 */
extern int inputLineNumber;

/**
 * Variable to enable/disable debug mode<BR>
 * Set to a value unequal to <code>0</code> to enable debug output
 */
int debug = 0;

/**
 * Main application entry point.<BR>
 * Uses input from STDIN and forwards it to the scanner for processing.
 */
int main(void)
{
    yyparse();
    printSymbolTable();
    return 0;
}

/**
 * This function is called by the parser if an error has been detected while
 * parsing the input data (e.g. syntax error).
 * @param str The error message to be printed.
 */
void yyerror(const char* str)
{
    fprintf(stderr, "Error while parsing input file (Line: %d): %s\n",\
            inputLineNumber, str);
}

char* helperVariableCounter(){
    char* h = malloc(sizeof(char)*10);
    sprintf(h, "_h%i", helperCounter);
    helperCounter++;
    return h;
}
dataType getType(symbolTableEntry *firstEntry, symbolTableEntry *secondEntry) {

	if (firstEntry->type == BOOLEAN && secondEntry->type == BOOLEAN) {
		return BOOLEAN;
	}

	if (firstEntry->type == INTEGER && secondEntry->type == INTEGER) {
		return INTEGER;
	}

	return REAL;
}

int hasTypeConflict(dataType firstEntry, dataType secondEntry){
  if(firstEntry==secondEntry) return 0;
    switch (firstEntry) {
      case REAL:
      if (secondEntry==BOOLEAN) {
        return 1;
      }
      break;
      case INTEGER:
      if (secondEntry==BOOLEAN || secondEntry==REAL) {
        return 1;
      }
      break;
      case BOOLEAN:
      if (secondEntry==REAL || secondEntry==INTEGER) {
        return 1;
      }
      break;

      return 0;
    }

}
