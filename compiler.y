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
%start S
%token MINUS PLUS TIMES DIV MOD INCREASE DECREASE
%token COMMENT SET SEPERATE COM EXIT
%token SMALLEQ BIGEQ SMALL BIG EQ NOTEQ
%token AND OR NOT
%token INT FLOAT BOOL
%token INTVAL FLOATVAL BOOLVAL VAR
%token IF THEN ELSE WHILE DO END
%token OBR CBR

%type <character> VAR
%type <integer> INTVAL
%type <floating> FLOATVAL
%type <boolval> BOOLVAL

%left PLUS MINUS
%left TIMES DIV

%%    // grammar rules

S:  ID SET E SEPERATE S
  | DECLERATE ID SET E SEPERATE S
  | DECLERATE ID COM R
  | DECLERATE ID
  | IF BR THEN EL END
  | WHILE BR DO S END
  | INCREASE E SEPERATE
  | DECREASE E SEPERATE;

EL: S
  | S ELSE S;

R:  ID
  | ID COM R;

E:  E PLUS E
  | E MINUS E
  | E TIMES E
  | E DIV E
  | E MOD E
  | INCREASE E
  | DECREASE E
  | E
  | NUM
  | ID;
BR:  OBR E CBR
   | OBR BR CBR;

%%
