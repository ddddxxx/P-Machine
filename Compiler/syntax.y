%{

#include "ast.h"

extern int yylex();
extern int yyerror(const char* msg);

%}

%error-verbose
%locations
%glr-parser
%expect 1

%union {
    struct ast_node *node_value;
}

%type  <node_value> program     sub_program
%type  <node_value> const_dec   const_list      const_elem
%type  <node_value> var_dec     var_list        var_elem
%type  <node_value> proc_dec    proc_list       proc_elem
%type  <node_value> para_dec    para_list       para_elem
%type  <node_value> arg_dec     arg_list        arg_elem
%type  <node_value> statement   assign_stat     if_stat     while_stat
%type  <node_value> call_stat   read_stat       write_stat  block_stat
%type  <node_value> stat_list   condition       expr

%token <node_value> NUMBER      IDENT

%token CONST_SYM    VAR_SYM     PROC_SYM    BEGIN_SYM   END_SYM
%token IF_SYM       THEN_SYM    WHILE_SYM   DO_SYM
%token ODD_SYM      ASSIGN_OP   CALL_SYM    READ_SYM    WRITE_SYM
%left  ADD_OP       SUB_OP      MUL_OP      DIV_OP
%token LSS_OP       LEQ_OP      GTR_OP      GEQ_OP      NEQ_OP      EQU_OP
%token LP_SYM       RP_SYM      COMMA       SEMICOLON   PERIOD

%%

program
    : sub_program PERIOD {
        root = reduce(program, NODE_VALUE, 1, $1);
    }
    ;
sub_program
    : const_dec var_dec proc_dec statement {
        $$ = reduce(sub_program, NODE_VALUE, 4, $1, $2, $3, $4);
    }
    ;

const_dec
    : /* empty */ {
        $$ = NULL;
    }
    | CONST_SYM const_list SEMICOLON {
        $$ = $2 ? reduce(const_dec, NODE_VALUE, 1, $2) : NULL;
    }
    ;
const_list
    : const_elem {
        $$ = $1;
    }
    | const_list COMMA const_elem {
        $$ = combine($1, $3);
    }
    | error {
        $$ = NULL;
    }
    | const_list COMMA error {
        $$ = $1;
    }
    ;
const_elem
    : IDENT EQU_OP NUMBER {
        $$ = reduce(const_elem, NODE_VALUE, 2, $1, $3);
    }
    ;

var_dec
    : /* empty */ {
        $$ = NULL;
    }
    | VAR_SYM var_list SEMICOLON {
        $$ = $2 ? reduce(var_dec, NODE_VALUE, 1, $2) : NULL;
    }
    | VAR_SYM var_list {
        $$ = $2 ? reduce(var_dec, NODE_VALUE, 1, $2) : NULL;
    }
    ;
var_list
    : var_elem {
        $$ = $1;
    }
    | var_list COMMA var_elem {
        $$ = combine($1, $3);
    }
    | error {
        $$ = NULL;
    }
    | var_list COMMA error {
        $$ = $1;
    }
    ;
var_elem
    : IDENT {
        $$ = $1;
    }
    ;

proc_dec
    : /* empty */ {
        $$ = NULL;
    }
    | proc_list {
        $$ = $1 ? reduce(proc_dec, NODE_VALUE, 1, $1) : NULL;
    }
    ;
proc_list
    : proc_elem {
        $$ = $1;
    }
    | proc_list proc_elem {
        $$ = combine($1, $2);
    }
    ;
proc_elem
    : PROC_SYM IDENT para_dec SEMICOLON sub_program SEMICOLON {
        $$ = reduce(proc_elem, NODE_VALUE, 3, $2, $3, $5);
    }
    ;
para_dec
    : LP_SYM para_list RP_SYM {
        $$ = reduce(para_dec, NODE_VALUE, 1, $2);
    }
    ;
para_list
    : /* empty */ {
        $$ = NULL;
    }
    | para_elem {
        $$ = $1;
    }
    | para_list COMMA para_elem {
        $$ = combine($1, $3);
    }
    | error {
        $$ = NULL;
    }
    | para_list COMMA error {
        $$ = $1;
    }
    ;
para_elem
    : IDENT {
        $$ = $1;
    }
    ;
arg_dec
    : LP_SYM arg_list RP_SYM {
        $$ = reduce(arg_dec, NODE_VALUE, 1, $2);
    }
    ;
arg_list
    :  /* empty */ {
        $$ = NULL;
    }
    | arg_elem {
        $$ = $1;
    }
    | arg_list COMMA arg_elem {
        $$ = combine($1, $3);
    }
    | error {
        $$ = NULL;
    }
    | arg_list COMMA error {
        $$ = $1;
    }
    ;
arg_elem
    : expr {
        $$ = $1;
    }
    ;
statement
    : /* empty */ {
        $$ = NULL;
    }
    | assign_stat {
        $$ = $1;
    }
    | if_stat {
        $$ = $1;
    }
    | while_stat {
        $$ = $1;
    }
    | call_stat {
        $$ = $1;
    }
    | read_stat {
        $$ = $1;
    }
    | write_stat {
        $$ = $1;
    }
    | block_stat {
        $$ = $1;
    }
    ;

assign_stat
    : IDENT ASSIGN_OP expr {
        $$ = reduce(assign_stat, NODE_VALUE, 2, $1, $3);
    }
    | IDENT EQU_OP expr {
        yyerror("assigning using '='");
        $$ = reduce(assign_stat, NODE_VALUE, 2, $1, $3);
    }
    ;
if_stat
    : IF_SYM condition THEN_SYM statement {
        $$ = reduce(if_stat, NODE_VALUE, 2, $2, $4);
    }
    ;
while_stat
    : WHILE_SYM condition DO_SYM statement {
        $$ = reduce(while_stat, NODE_VALUE, 2, $2, $4);
    }
    ;
call_stat
    : CALL_SYM IDENT arg_dec {
        $$ = reduce(call_stat, NODE_VALUE, 2, $2, $3);
    }
    ;
read_stat
    : READ_SYM IDENT {
        $$ = reduce(read_stat, NODE_VALUE, 1, $2);
    }
    ;
write_stat
    : WRITE_SYM expr {
        $$ = reduce(write_stat, NODE_VALUE, 1, $2);
    }
    ;
block_stat
    : BEGIN_SYM stat_list END_SYM {
        $$ = reduce(block_stat, NODE_VALUE, 1, $2);
    }
    ;
stat_list
    : statement {
        $$ = $1;
    }
    | stat_list SEMICOLON statement {
        $$ = combine($1, $3);
    }
    | error {
        $$ = NULL;
    }
    | stat_list SEMICOLON error {
        $$ = $1;
    }
    ;

condition
    : expr LSS_OP expr {
        $$ = reduce(condition, LSS_OP, 2, $1, $3);
    }
    | expr LEQ_OP expr {
        $$ = reduce(condition, LEQ_OP, 2, $1, $3);
    }
    | expr GTR_OP expr {
        $$ = reduce(condition, GTR_OP, 2, $1, $3);
    }
    | expr GEQ_OP expr {
        $$ = reduce(condition, GEQ_OP, 2, $1, $3);
    }
    | expr NEQ_OP expr {
        $$ = reduce(condition, NEQ_OP, 2, $1, $3);
    }
    | expr EQU_OP expr {
        $$ = reduce(condition, EQU_OP, 2, $1, $3);
    }
    ;
expr
    : expr ADD_OP expr {
        $$ = reduce(expr, ADD_OP, 2, $1, $3);
    }
    | expr SUB_OP expr {
        $$ = reduce(expr, SUB_OP, 2, $1, $3);
    }
    | expr MUL_OP expr {
        $$ = reduce(expr, MUL_OP, 2, $1, $3);
    }
    | expr DIV_OP expr {
        $$ = reduce(expr, DIV_OP, 2, $1, $3);
    }
    | LP_SYM expr RP_SYM {
        $$ = $2;
    }
    | NUMBER {
        $$ = $1;
    }
    | IDENT {
        $$ = $1;
    }
    ;

%%

int yyerror(const char* msg) {
    // TODO: error
    fprintf(stderr, "%d:%d %s\n", yylloc.first_line, yylloc.first_column, msg);
    return 0;
}
