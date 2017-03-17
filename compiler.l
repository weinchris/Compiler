%{

#include "../compiler.h"
#include "../symboltable.h"
#include "compiler.tab.h"

/**
 * Variable to track current line number.
 */
int inputLineNumber = 1;

%}

%option nounput

%%

%%
