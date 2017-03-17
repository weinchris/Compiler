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
\# {return COMMENT;}
-- {return DECREASE}
\+\+ {return INCREASE}
- {return MINUS;}
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

\= {return SET}
; {return SEPERATE;}
int {return INT;}
float {return FLOAT;}
bool {return BOOL};

if {return IF;}
then {return THEN:}
else {return ELSE;}
while {return WHILE;}
do {return DO;}
end {return END;}

exit {return EXIT;}

\-?[0-9]+\.[0-9]+ {yyval.floating=atof{yytext}; return FLOAT;}
\-?[1-9][0-9]* {yyval.integer=ato{yytext}; return INT;}
[A-Za-z][A-Za-z0-9]* {yyval.character=strdup(yytext); return VAR;}

%%
