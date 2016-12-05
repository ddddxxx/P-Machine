#ifndef ast_h
#define ast_h

#include <stdio.h>
#include <stdbool.h>
#include "syntax.tab.h"

#define NODE_VALUE 0
#define IDENT_TABLE_INIT_SIZE 100

typedef struct ast_node ast_node;
typedef enum   ast_type ast_type;

enum ast_type {
    none,           program,        sub_program,    const_dec,
    var_dec,        proc_dec,       para_dec,       arg_dec,
    const_elem,     proc_elem,      assign_stat,    if_stat,
    while_stat,     call_stat,      read_stat,      write_stat,
    block_stat,     condition,      expr,           num,
    id,
};

struct ast_node {
    ast_type type;
    int value;      // index of ident table for id, number for num, operator for condition and expr
    YYLTYPE loc;
    struct ast_node *next;
    struct ast_node *child;
};

bool is_statement(ast_node *node);
bool is_expr(ast_node *node);

extern ast_node *root;
ast_node* create_node(ast_type type, int value);
ast_node* reduce(int type, int value, int count, ...);
ast_node* combine(ast_node *head, ast_node *tail);
int count_child(ast_node *parent);

void traverse(ast_node *node, int indent);
void print_node(ast_node *node, int indent);

extern char **ident_table;
int  get_ident_index(char *ident);

#endif
