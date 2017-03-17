%{
//Prologue

#include "../compiler.h"
#include "../symboltable.h"
#include "../generator.h"
#include "../interpreter.h"
#include <stdio.h>

int main(int argc, char **argv);
void yyerror(char *message);
// Variable to track current line number  [defined in compiler.l]
extern int inputLineNumber;

%}
//Bison declarations

%error-verbose

// Define all tokens used in the scanner
%union {
  char* character;
	int integer;
	double floating;
  int boolval;
       }
%token MINUS PLUS TIMES DIV MOD INCREASE DECREASE
%token COMMENT SET SEPERATE EXIT
%token SMALLEQ BIGEQ SMALL BIG EQ NOTEQ
%token AND OR NOT
%token INT FLOAT BOOL
%token INTVAL FLOATVAL BOOLVAL VAR
%token IF THEN ELSE WHILE DO END
%token OBR CBR
%token TEST

%type <character> VAR
%type <integer> INTVAL
%type <floating> FLOATVAL
%type <boolval> BOOLVAL


%%    // grammar rules

  test:
        TEST;
%%
int main (int argc, char **argv) {
	yyparse();
	return 0;
}
void yyerror(char *message) {
	printf("\nSomething went wrong\n");
}
//Epilogue
