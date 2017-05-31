%{
//Prologue

#include "../compiler.h"
#include "../symboltable.h"
#include "../generator.h"
#include "../interpreter.h"
#include <stdio.h>

int main(int argc, char **argv);
void yyerror(const char* message);
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
%type <daType> TYPE BR
%type <tableEntry> NUM E DEC

%left AND OR
%right NOT INCREASE DECREASE
%left PLUS MINUS
%left TIMES DIV MOD
%left EQ NOTEQ BIG BIGEQ SMALL SMALLEQ


%%    // grammar rules

S:  VAR SET E SEPERATE S
  {
    if (! getEntryFromSymbolTable($1))
    {
        fprintf(stderr, "%s does not exist. Line: %d\n", $1, inputLineNumber);
        YYABORT;
    }else if(hasTypeConflict(getEntryFromSymbolTable($1)->type, $3->type)){
      fprintf(stderr, "%s has type conflict. Line: %d\n", $1, inputLineNumber);
      YYABORT;
    }
  }
  | INCREASE E SEPERATE S
  | DECREASE E SEPERATE S
  | DEC SEPERATE S
  | IF BR THEN S EL END SEPERATE S
  {
    if($2!=BOOLEAN){
    fprintf(stderr, "Boolean expected at if statement. Line: %d\n", inputLineNumber);
    YYABORT;}
  }
  | WHILE BR DO S END SEPERATE S
  {
    if($2!=BOOLEAN){
    fprintf(stderr, "Boolean expected at while statement. Line: %d\n", inputLineNumber);
    YYABORT;}
  }
  | EN
  |;

EL: ELSE S
  |;

E:  E BIG E
{
      if ($1->type == BOOLEAN || $3->type == BOOLEAN)
      {
          if ($1->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }
  | E BIGEQ E
  {
		if ($1->type == BOOLEAN || $3->type == BOOLEAN)
		{
			if ($1->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
			if ($3->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
			YYABORT;
		}
		$$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
	}
  | E SMALL E
  {
		if ($1->type == BOOLEAN || $3->type == BOOLEAN)
		{
			if ($1->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
			if ($3->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
			YYABORT;
		}
		$$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
	}
  | E SMALLEQ E
  {
		if ($1->type == BOOLEAN || $3->type == BOOLEAN)
		{
			if ($1->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
			if ($3->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
			YYABORT;
		}
		$$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
	}
  | E EQ E { $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);}
  | E NOTEQ E { $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);}
  | E AND E
  {
      if (! ($1->type == BOOLEAN) || ! ($3->type == BOOLEAN))
      {
          if ($1->type == REAL)
              fprintf(stderr, "%s is of type float. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == REAL)
              fprintf(stderr, "%s is of type float. Line: %d\n", $3->name, inputLineNumber);
          if ($1->type == INTEGER)
              fprintf(stderr, "%s is of type integer. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == INTEGER)
              fprintf(stderr, "%s is of type float. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }

  | E OR E
  {
      if (! ($1->type == BOOLEAN) || ! ($3->type == BOOLEAN))
      {
          if ($1->type == REAL)
              fprintf(stderr, "%s is of type float. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == REAL)
              fprintf(stderr, "%s is of type float. Line: %d\n", $3->name, inputLineNumber);
          if ($1->type == INTEGER)
              fprintf(stderr, "%s is of type integer. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == INTEGER)
              fprintf(stderr, "%s is of type float. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }
  | NOT E
  {
      if ( $2->type != BOOLEAN)
      {
          if ($2->type == REAL)
              fprintf(stderr, "%s is of type float. Line: %d\n", $2->name, inputLineNumber);
          if ($2->type == INTEGER)
              fprintf(stderr, "%s is of type integer. Line: %d\n", $2->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), $2->type, inputLineNumber);
  }
  | E PLUS E
  {
		if ($1->type == BOOLEAN || $3->type == BOOLEAN)
		{
			if ($1->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
			if ($3->type == BOOLEAN)
				fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
			YYABORT;
		}
		$$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
	}
  | E MINUS E
  {
      if ($1->type == BOOLEAN || $3->type == BOOLEAN)
      {
          if ($1->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }
  | E TIMES E
  {
      if ($1->type == BOOLEAN || $3->type == BOOLEAN)
      {
          if ($1->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }
  | E DIV E
  {
      if ($1->type == BOOLEAN || $3->type == BOOLEAN)
      {
          if ($1->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);
          if ($3->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }
  | E MOD E
  {
      if ($1->type == BOOLEAN || $3->type == BOOLEAN)
      {
          if ($1->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $1->name, inputLineNumber);

          if ($3->type == BOOLEAN)
              fprintf(stderr, "%s is of type boolean. Line: %d\n", $3->name, inputLineNumber);
          YYABORT;
      }
      $$ = addEntryToSymbolTable(getName(), getType($1, $3), inputLineNumber);
  }
  | NUM {$$ = $1;}
  | VAR
  {
      if (! getEntryFromSymbolTable($1))
      {
          fprintf(stderr, "%s does not exist. Line: %d\n", $1, inputLineNumber);
          YYABORT;
      } else $$ = (getEntryFromSymbolTable($1));
  }
  | INCREASE E { $$ = $2;}
  | DECREASE E { $$ = $2;};

BR: OBR E Z CBR {$$=$2->type;};

Z: BR
  |;

DEC: TYPE VAR
  {
      if (getEntryFromSymbolTable($2))
      {
          fprintf(stderr, "%s already exists. Line: %d\n", $2, inputLineNumber);
          YYABORT;
      } else $$ = addEntryToSymbolTable($2, $1, inputLineNumber);
  }
  | TYPE VAR SET E
  {
    if (getEntryFromSymbolTable($2))
    {
        fprintf(stderr, "%s already exists. Line: %d\n", $2, inputLineNumber);
        YYABORT;
    } else if(hasTypeConflict($1,$4->type)){
      fprintf(stderr, "%s has type conflict. Line: %d\n", $2, inputLineNumber);
      YYABORT;
    }
    else $$ = addEntryToSymbolTable($2, $1, inputLineNumber);
  }
  | DEC COM VAR
  {
    if (getEntryFromSymbolTable($3))
    {
        fprintf(stderr, "%s already exists. Line: %d\n", $3, inputLineNumber);
        YYABORT;
    } else $$ = addEntryToSymbolTable($3, $1->type, inputLineNumber);
  }
  | DEC COM VAR SET E
  {
    if (getEntryFromSymbolTable($3))
    {
        fprintf(stderr, "%s already exists. Line: %d\n", $3, inputLineNumber);
        YYABORT;
    } else if(hasTypeConflict($1->type, $5->type)){
      fprintf(stderr, "%s has type conflict. Line: %d\n", $3, inputLineNumber);
      YYABORT;
    }
    else $$ = addEntryToSymbolTable($3, $1->type, inputLineNumber);
  };

TYPE: INT {$$ = INTEGER;}
  | FLOAT {$$ = REAL;}
  | BOOL {$$ = BOOLEAN;};

NUM: INTVAL {$$ = addEntryToSymbolTable(getName(), INTEGER, inputLineNumber);}
  | FLOATVAL {$$ = addEntryToSymbolTable(getName(), REAL, inputLineNumber);}
  | BOOLVAL {$$ = addEntryToSymbolTable(getName(), BOOLEAN, inputLineNumber);};

EN: EXIT SEPERATE
  | EXIT VAR SEPERATE
  {
      if (! getEntryFromSymbolTable($2))
      {
          fprintf(stderr, "%s does not exist. Line: %d\n", $2, inputLineNumber);
          YYABORT;
      }
  }
  | EXIT BR SEPERATE;

%%
