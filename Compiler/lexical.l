%{

#include "ast.h"
#include "syntax.tab.h"

static int yycolumn  = 1;
static void update_loc() {
    yylloc.first_line   = yylineno;
    yylloc.first_column = yycolumn;
    yycolumn += yyleng;
    yylloc.last_line   = yylineno;
    yylloc.last_column = yycolumn;
}

#define YY_USER_ACTION update_loc();

extern int yywrap();

%}

%option yylineno
%start  COMMENT

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  ({letter}|"_")({letter}|{number}|"_")*
blank       [\t\x0B\f\r\x20]+

%%

<INITIAL>"const"        { return CONST_SYM; }
<INITIAL>"var"          { return VAR_SYM; }
<INITIAL>"procedure"    { return PROC_SYM; }
<INITIAL>"begin"        { return BEGIN_SYM; }
<INITIAL>"end"          { return END_SYM; }
<INITIAL>"odd"          { return ODD_SYM; }
<INITIAL>"if"           { return IF_SYM; }
<INITIAL>"then"         { return THEN_SYM; }
<INITIAL>"call"         { return CALL_SYM; }
<INITIAL>"while "       { return WHILE_SYM; }
<INITIAL>"do"           { return DO_SYM; }
<INITIAL>"read"         { return READ_SYM; }
<INITIAL>"write"        { return WRITE_SYM; }

<INITIAL>"+"            { return ADD_OP; }
<INITIAL>"-"            { return SUB_OP; }
<INITIAL>"*"            { return MUL_OP; }
<INITIAL>"/"            { return DIV_OP; }
<INITIAL>"<"            { return LSS_OP; }
<INITIAL>"<="           { return LEQ_OP; }
<INITIAL>">"            { return GTR_OP; }
<INITIAL>">="           { return GEQ_OP; }
<INITIAL>"<>"           { return NEQ_OP; }
<INITIAL>"="            { return EQU_OP; }
<INITIAL>":="           { return ASSIGN_OP; }
<INITIAL>"("            { return LP_SYM; }
<INITIAL>")"            { return RP_SYM; }

<INITIAL>{number}       { yylval.node_value = create_node(num, (int)atol(yytext)); return NUMBER; }
<INITIAL>{identifier}   { yylval.node_value = create_node(id, get_ident_index(yytext)); return IDENT; }
<INITIAL>{blank}

<INITIAL>","            { return COMMA; }
<INITIAL>";"            { return SEMICOLON; }
<INITIAL>"."            { return PERIOD; }

<INITIAL>"\n"           { yycolumn = 1; }

<INITIAL>"//"[^\n]*
<INITIAL>"/*"           { BEGIN COMMENT; }

<COMMENT>"*/"           { BEGIN INITIAL; }
<COMMENT>[^\n]
<COMMENT>"\n"           { yycolumn = 1; }

%%

int yywrap() {
    return 1;
}
