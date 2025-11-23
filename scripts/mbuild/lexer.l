%{
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);   /* ensure correct prototype */
%}

%option noyywrap
%x LIST

%%

"#".*                       { /* ignore comments */ }
[ \t\r\n]                   { /* ignore whitespace */ }
"."[a-z]+                   { yylval.sval = strdup(yytext); return T_OP; }
"("                         { BEGIN(LIST); return '('; }
";"                         { return ';'; }
[a-zA-Z0-9_\-]+             { yylval.sval = strdup(yytext); return T_IDENT; }
.                           { fprintf(stderr, "Unexpected token: %s\n", yytext); }

<LIST>\n                    { /* ignore newline */ }
<LIST>[ \t\r\n]             { /* ignore whitespace */ }
<LIST>")"                   { BEGIN(INITIAL); return ')'; }
<LIST>","                   { return ','; }
<LIST>[a-zA-Z0-9_\.\/\-]+   { yylval.sval = strdup(yytext); return T_LIST_IDENT; }
<LIST>.                     { fprintf(stderr, "Unexpected token: %s\n", yytext); }

%%