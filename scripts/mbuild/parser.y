%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
int yyerror(const char *s);
%}

%code requires {
    #include "ast.h"
}

/* declare tokens */
%union {
    int ival;
    char* sval;
    struct AstList list;
}

%token <sval> T_IDENT T_LIST_IDENT T_OP
%type <ival> op
%type <list> list list_items qualifier_list qualifier_list_items

%%

program:
    /* empty */
    | program statement
    ;

statement:
    T_IDENT op '(' list ')' qualifier_list ';' { ast_insert_node($1, $2, &$4, &$6); }
    ;

op:
    /* empty */ { $$ = OP_ADD; }
    | T_OP { $$ = ast_decode_op($1); }
    ;

list:
    /* empty */ {
        $$.count  = 0;
        $$.values = NULL;
    }
    | list_items {$$ = $1; }
    ;

list_items: 
    T_LIST_IDENT {
        $$.count = 1;
        $$.values = malloc(sizeof(char*));
        $$.values[0] = $1;
    }
    | list_items ',' T_LIST_IDENT {
        $1.count++;
        $1.values = realloc($1.values, sizeof(char*) * $1.count);
        $1.values[$1.count - 1] = $3;
        $$ = $1;
    }
    ;

qualifier_list:
    /* empty */ {
        $$.count = 0;
        $$.values = NULL;
    }
    | qualifier_list_items { $$ = $1; }
    ;

qualifier_list_items:
    T_IDENT {
        $$.count = 1;
        $$.values = malloc(sizeof(char*));
        $$.values[0] = $1;
    }
    | qualifier_list_items T_IDENT {
        $1.count++;
        $1.values = realloc($1.values, sizeof(char*) * $1.count);
        $1.values[$1.count - 1] = $2;
        $$ = $1;
    }
    ;

%%

int yyerror(const char *s)
{
    fprintf(stderr, "error: %s\n", s);
    return 0;
}
