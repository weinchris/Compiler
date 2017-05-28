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
\#.* {}
\= {return SET;}
; {return SEPERATE;}
, {return COM;}
exit {return EXIT;}

-- {return DECREASE;}
\+\+ {return INCREASE;}
\- {return MINUS;}
\+ {return PLUS;}
\* {return TIMES;}
\/ {return DIV;}
% {return MOD;}

\<\= {return SMALLEQ;}
>\= {return BIGEQ;}
\< {return SMALL;}
\> {return BIG;}
\=\= {return EQ;}
\!\= {return NOTEQ;}

and {return AND;}
or {return OR;}
not {return NOT;}

\( {return OBR;}
\) {return CBR;}

int {return INT;}
float {return FLOAT;}
bool {return BOOL;}

if {return IF;}
then {return THEN;}
else {return ELSE;}
while {return WHILE;}
do {return DO;}
end {return END;}
true {yylval.boolval=1; return BOOLVAL;}
false {yylval.boolval=0; return BOOLVAL;}
\-?[0-9]+\.[0-9]+ {yylval.floating=atof(yytext); return FLOATVAL;}
\-?[1-9][0-9]*|0 {yylval.integer=atoi(yytext); return INTVAL;}
[A-Za-z][A-Za-z0-9]* {yylval.character=strdup(yytext); return VAR;}

\n {++inputLineNumber;}
[ \t]  {}

.  { printf("Invalid identifier. Line: %d \n", inputLineNumber); yyerror;}

%%
