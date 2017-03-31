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

S:  VAR SET E SEPERATE S
  | DECLERATE VAR SET E SEPERATE S
  | DECLERATE VAR COM R S
  | DECLERATE VAR S
  | IF BR THEN EL END S
  | WHILE BR DO S END S
  | INCREASE E SEPERATE S
  | DECREASE E SEPERATE S
  | EN;

EL: S
  | S ELSE S;

R:  VAR
  | VAR COM R;

E:  E PLUS E
  | E MINUS E
  | E TIMES E
  | E DIV E
  | E MOD E
  | INCREASE E
  | DECREASE E
  | E
  | NUM
  | VAR;

BR:  OBR E CBR
   | OBR BR CBR;

DECLERATE: INT
  | FLOAT
  | BOOL;

NUM: INTVAL
  | FLOATVAL
  | BOOLVAL;

EN: EXIT SEPERATE
  | EXIT VAR SEPERATE;

%%
