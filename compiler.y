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
%token SET SEPERATE COM EXIT
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

%left AND OR
%right NOT
%left PLUS MINUS
%left TIMES DIV MOD
%left EQ NOTEQ BIG BIGEQ SMALL SMALLEQ


%%    // grammar rules

S:  VAR SET E SEPERATE S
  | DEC R X SEPERATE S
  | IF BR THEN S EL END SEPERATE S
  | WHILE BR DO S END SEPERATE S
  | INCREASE E SEPERATE S
  | DECREASE E SEPERATE S
  | EN
  |;

X: SET E
  |;

EL: ELSE S
  |;

R:  VAR
  | VAR COM R;

E:  ID OPS E
  | ID;

ID: NUM
  | VAR
  | INCREASE ID
  | DECREASE ID;

BR:  OBR E Z CBR;

Z: BR
  |;

OPS: BIG
  | BIGEQ
  | SMALL
  | SMALLEQ
  | EQ
  | NOTEQ
  | PLUS
  | MINUS
  | TIMES
  | DIV
  | MOD
  | NOT
  | AND
  | OR;

DEC: INT
  | FLOAT
  | BOOL;

NUM: INTVAL
  | FLOATVAL
  | BOOLVAL;

EN: EXIT SEPERATE
  | EXIT VAR SEPERATE;

%%
