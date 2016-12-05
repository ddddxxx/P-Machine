#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>
#include "ast.h"

#define max(a,b) ((a)>(b) ? (a) : (b)
#define min(a,b) ((a)<(b) ? (a) : (b)

extern FILE *ast_out;

ast_node *root = NULL;

ast_node* create_node(ast_type type, int value) {
    ast_node *node = (ast_node *)malloc(sizeof(ast_node));
    node->type     = type;
    node->value    = value;
    node->loc      = yylloc;
    node->next     = NULL;
    node->child    = NULL;
    return node;
}

ast_node* reduce(int type, int value, int count, ...) {
    ast_node *result = create_node(type, value);
    va_list argp;
    va_start(argp, count);
    ast_node *head = NULL;
    ast_node *tail = NULL;
    ast_node *current = NULL;
    for (int i=0; i<count; i++) {
        current = va_arg(argp, ast_node*);
        if (current == NULL) {
            continue;
        }
        if (!tail) {
            tail = current;
            head = current;
        } else {
            tail->next = current;
            tail = tail->next;
        }
    }
    if (head) {
        result->loc.first_line   = head->loc.first_line;
        result->loc.first_column = head->loc.first_column;
        result->loc.last_line    = tail->loc.last_line;
        result->loc.last_column  = tail->loc.last_column;
    }
    result->child = head;
    va_end(argp);
    return result;
}

ast_node* combine(ast_node *head, ast_node *tail) {
    if (!head) {
        head = tail;
        return head;
    } else if (!tail) {
        return head;
    }
    ast_node *tmp=head;
    while (tmp->next) tmp=tmp->next;
    tmp->next = tail;
    return head;
}

int count_child(ast_node *parent) {
    if (!parent) {
        return 0;
    }
    int result = 0;
    ast_node *child = parent->child;
    while(child) {
        result++;
        child = child->next;
    }
    return result;
}

bool is_statement(ast_node *node) {
    int type = node->type;
    return type>=assign_stat && type<=block_stat;
}

bool is_expr(ast_node *node) {
    int type = node->type;
    return type==expr || type==id || type==num;
}

void traverse(ast_node *node, int indent) {
    if(node) {
        ast_node *tmp = node;
        while (tmp) {
            print_node(tmp, indent);
            traverse(tmp->child, indent+1);
            tmp = tmp -> next;
        }
    }
}

void print_node(ast_node *node, int indent) {
    // fprintf(ast_out, "%-3d%-3d%-3d%-3d", node->loc.first_line, node->loc.first_column, node->loc.last_line, node->loc.last_column);

    for(int i = 0;i < indent; i++) {
        fprintf(ast_out, "  ");
    }

    switch (node->type) {
        case none:
            fprintf(ast_out, "%s\n", "err_type");
            break;
        case program:
            fprintf(ast_out, "%s\n", "program");
            break;
        case sub_program:
            fprintf(ast_out, "%s\n", "sub_program");
            break;
        case const_dec:
            fprintf(ast_out, "%s\n", "const");
            break;
        case var_dec:
            fprintf(ast_out, "%s\n", "var");
            break;
        case proc_dec:
            fprintf(ast_out, "%s\n", "procs");
            break;
        case para_dec:
            fprintf(ast_out, "%s\n", "paras");
            break;
        case arg_dec:
            fprintf(ast_out, "%s\n", "args");
            break;
        case const_elem:
            fprintf(ast_out, "%s\n", "=");
            break;
        case proc_elem:
            fprintf(ast_out, "%s\n", "proc");
            break;
        case assign_stat:
            fprintf(ast_out, "%s\n", ":=");
            break;
        case if_stat:
            fprintf(ast_out, "%s\n", "if");
            break;
        case while_stat:
            fprintf(ast_out, "%s\n", "while");
            break;
        case call_stat:
            fprintf(ast_out, "%s\n", "call");
            break;
        case read_stat:
            fprintf(ast_out, "%s\n", "read");
            break;
        case write_stat:
            fprintf(ast_out, "%s\n", "write");
            break;
        case block_stat:
            fprintf(ast_out, "%s\n", "block");
            break;
        case condition:
            switch (node->value) {
                case LSS_OP:
                    fprintf(ast_out, "%s\n", "<");
                    break;
                case LEQ_OP:
                    fprintf(ast_out, "%s\n", "<=");
                    break;
                case GTR_OP:
                    fprintf(ast_out, "%s\n", ">");
                    break;
                case GEQ_OP:
                    fprintf(ast_out, "%s\n", ">=");
                    break;
                case NEQ_OP:
                    fprintf(ast_out, "%s\n", "<>");
                    break;
                case EQU_OP:
                    fprintf(ast_out, "%s\n", "=");
                    break;
            }
            break;
        case expr:
            switch (node->value) {
                case ADD_OP:
                    fprintf(ast_out, "%s\n", "+");
                    break;
                case SUB_OP:
                    fprintf(ast_out, "%s\n", "-");
                    break;
                case MUL_OP:
                    fprintf(ast_out, "%s\n", "*");
                    break;
                case DIV_OP:
                    fprintf(ast_out, "%s\n", "/");
                    break;
            }
            break;
        case num:
            fprintf(ast_out, "%d\n", node->value);
            break;
        case id:
            fprintf(ast_out, "%s\n", ident_table[node->value]);
            break;
        }
}

char **ident_table   = NULL;
int ident_table_size = 0;
int ident_table_num  = 0;
int get_ident_index(char *ident) {
    if (!ident_table) {
        ident_table_size = IDENT_TABLE_INIT_SIZE;
        ident_table = malloc(ident_table_size * sizeof(char*));
        assert(ident_table);
    }

    for (int i=0; i<ident_table_num; i++) {
        if (!strcmp(ident, ident_table[i])) {
            return i;
        }
    }

    ident_table[ident_table_num] = strdup(ident);
    ident_table_num++;

    if (ident_table_num >= ident_table_size) {
        ident_table_size *= 2;
        ident_table = realloc(ident_table, ident_table_size * sizeof(char*));
        assert(ident_table);
    }
    return ident_table_num-1;
}
