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
  symbolTableEntry* tableEntry;
  dataType daType;
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


%type <daType> TYPE DEC

%left AND OR
%right NOT
%left PLUS MINUS
%left TIMES DIV MOD
%left EQ NOTEQ BIG BIGEQ SMALL SMALLEQ


%%    // grammar rules

S:  VAR SET E SEPERATE S
  | INC SEPERATE S
  | DEC SEPERATE S
  | IF BR THEN S EL END SEPERATE S
  | WHILE BR DO S END SEPERATE S
  | EN
  |;

EL: ELSE S
  |;

E:  E BIG E
  | E BIGEQ E
  | E SMALL E
  | E SMALLEQ E
  | E EQ E
  | E NOTEQ E
  | E PLUS E
  | E MINUS E
  | E TIMES E
  | E DIV E
  | E MOD E
  | E AND E
  | E OR E
  | E NOT E
  | NUM
  | VAR
  | INC;
INC:
  | INCREASE NUM
  | DECREASE NUM
  | INCREASE VAR
  | DECREASE VAR;

BR: OBR E Z CBR;

Z: BR
  |;

DEC: TYPE VAR {addEntryToSymbolTable($2, $1, inputLineNumber);}
  | TYPE VAR SET E {addEntryToSymbolTable($2, $1, inputLineNumber);}
  | DEC COM VAR {addEntryToSymbolTable($3, $1, inputLineNumber);}
  | DEC COM VAR SET E {addEntryToSymbolTable($3, $1, inputLineNumber);};

TYPE: INT {$$ = INTEGER;}
  | FLOAT {$$ = REAL;}
  | BOOL {$$ = BOOLEAN;};

NUM: INTVAL (addEntryToSymbolTable(helperVariableCounter, INTEGER, inputLineNumber))
  | FLOATVAL (addEntryToVariableTable
  | BOOLVAL (addEntryToVariableTable());

EN: EXIT SEPERATE
  | EXIT VAR SEPERATE
  | EXIT BR SEPERATE;

%%
