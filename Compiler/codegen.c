#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "codegen.h"
#include "syntax.tab.h"

#define AUTO_SET_VALUE 0

extern FILE *code_out;
extern FILE *raw_out;

instruction *code = NULL;
int code_loc = 0;
int code_size = 0;

symbol_table *symtbl = NULL;

void gen(mnemonic f, int l, int a) {
    if (code == 0) {
        code_loc  = 0;
        code_size = CODE_INIT_SIZE;
        code      = malloc(code_size * sizeof(instruction));
        assert(code);
    }

    code[code_loc].f = f;
    code[code_loc].l = l;
    code[code_loc].a = a;
    code_loc++;

    if (code_loc >= code_size) {
        code_size *= 2;
        code = realloc(code, code_size * sizeof(instruction));
        assert(code);
    }
}

char mnemonic_str[][4] = {"LIT", "OPR", "LOD", "STO", "CAL", "RET", "INC", "JMP", "JPC", "SI", "SO"};
void print_code() {
    for (int i=0; i<code_loc; i++) {
        int raw = (code[i].f<<24) | ((code[i].l<<16)&0x00ff0000) | (code[i].a&0x0000ffff);
        fprintf(code_out, "%4d  %08x  ; ", i, raw);
        fprintf(code_out, "%-3s %2d, %d\n", mnemonic_str[code[i].f], code[i].l, code[i].a);
    }
}

void print_raw_code() {
    fprintf(raw_out, "v2.0 raw\n");
    for (int i=0; i<code_loc; i++) {
        int raw = (code[i].f<<24) | ((code[i].l<<16)&0x00ff0000) | (code[i].a&0x0000ffff);
        fprintf(raw_out, "%08x\n", raw);
    }
}

symbol *creat_sym(symbol_type type, int name, int value) {
    assert(symtbl);

    // symbol *s = search_sym(name);
    // TODO: sym redec

    symtbl->syms[symtbl->num].type = type;
    symtbl->syms[symtbl->num].name = name;
    if (type==variable && value==AUTO_SET_VALUE) {
        symtbl->syms[symtbl->num].value = symtbl->size_to_alloc;
        symtbl->size_to_alloc++;
    } else {
        symtbl->syms[symtbl->num].value = value;
    }
    symtbl->syms[symtbl->num].layer = symtbl->layer;
    symtbl->num++;

    if (symtbl->num >= symtbl->size) {
        symtbl->size *= 2;
        symtbl->syms = realloc(symtbl->syms, symtbl->size * sizeof(symbol));
        assert(symtbl->syms);
    }

    return &(symtbl->syms[symtbl->num-1]);
}

void creat_sym_table() {
    symbol_table *table = malloc(sizeof(symbol_table));
    assert(table);
    table->num = 0;
    table->size = SYM_TABLE_INIT_SIZE;
    table->size_to_alloc = 4;   // space for the Static Link, Dynamic Link, and Return Address
    table->syms = malloc(table->size * sizeof(symbol));
    table->layer = symtbl ? symtbl->layer+1 : 0;
    assert(table->syms);
    table->outer = symtbl;
    symtbl = table;
}

void deatroy_current_sym_table() {
    if (symtbl) {
        symbol_table *tmp = symtbl;
        symtbl = symtbl->outer;
        free(tmp->syms);
        free(tmp);
    }
}

symbol *search_sym(int name) {
    for (symbol_table *t=symtbl; t; t=t->outer) {
        for (int i=0; i<t->num; i++) {
            if (t->syms[i].name == name) {
                return &(t->syms[i]);
            }
        }
    }
    // symbol undefine
    return NULL;
}

void gen_node(ast_node *node) {
    assert(node);
    switch (node->type) {
        case program: {
            assert(node->child);
            ast_node *content = node->child;
            assert(content->type==sub_program);
            creat_sym_table();              // creat a new symbol table
            gen_node(content);
        }
        break;
        case sub_program: {
            assert(node->child);
            ast_node *consts=NULL, *vars=NULL, *procs=NULL;
            ast_node *tmp=node->child;

            int jmp_code = code_loc;        // save code address
            gen(JMP, 0, 0);                 // temp jump target

            // const declaration
            if (tmp->type == const_dec) {
                consts = tmp;
                gen_node(consts);
                tmp = tmp->next;
                assert(tmp);
            }

            // var declaration
            if (tmp->type == var_dec) {
                vars = tmp;
                gen_node(vars);
                tmp = tmp->next;
                assert(tmp);
            }

            // procedure declaration
            if (tmp->type == proc_dec) {
                procs = tmp;
                gen_node(procs);
                tmp = tmp->next;
                assert(tmp);
            }

            // body
            assert(is_statement(tmp));
            if (code_loc == jmp_code+1) {
                code_loc --;
            } else {
                code[jmp_code].a = code_loc;    // jump target backfill
            }
            gen(INC, 0, symtbl->size_to_alloc); // allocate space for procedure
            gen_node(tmp);
            gen(RET, 0, 0);                     // return from procedure

            deatroy_current_sym_table();        // deatroy current symbol table
        }
        break;
        case const_dec: {
            assert(node->child);
            ast_node *tmp=node->child;
            while (tmp) {
                assert(tmp->type==const_elem);
                gen_node(tmp);
                tmp = tmp->next;
            }
        }
        break;
        case var_dec: {
            assert(node->child);
            ast_node *tmp=node->child;
            while (tmp) {
                assert(tmp->type==id);

                symbol *sym = search_sym(tmp->value);
                if (sym && sym->layer == symtbl->layer && sym->type == variable) {
                    // TODO: variable redeclaration
                    continue;
                }

                creat_sym(variable, tmp->value, AUTO_SET_VALUE);
                tmp = tmp->next;
            }
        }
        break;
        case proc_dec: {
            assert(node->child);
            ast_node *tmp=node->child;
            while (tmp) {
                assert(tmp->type==proc_elem);
                gen_node(tmp);
                tmp = tmp->next;
            }
        }
        break;
        case const_elem: {
            assert(node->child);
            ast_node *name=node->child;
            ast_node *value=name->next;
            assert(name->type==id && value->type==num);

            symbol *sym = search_sym(name->value);
            if (sym && sym->layer == symtbl->layer && sym->type == constant) {
                // TODO: constant redeclaration
                break;
            }
            creat_sym(constant, name->value, value->value);
        }
        break;
        case proc_elem: {
            assert(node->child);
            ast_node *name=node->child;
            ast_node *paras=name->next;
            ast_node *body=paras->next;
            assert(name->type==id && paras->type==para_dec && body->type==sub_program);

            symbol *sym = search_sym(name->value);
            if (sym && sym->layer == symtbl->layer && sym->type == procedure) {
                // TODO: procedure redeclaration
                break;
            }
            creat_sym(procedure, name->value, code_loc);

            creat_sym_table();              // creat a new symbol table
            int para_num=count_child(paras);
            ast_node *para=paras->child;
            for (int i=para_num; i>0; i--) {
                assert(para->type==id);
                creat_sym(variable, para->value, -i);
                para = para->next;
            }

            gen_node(body);
        }
        break;
        case assign_stat: {
            assert(node->child);
            ast_node *var_id=node->child;
            ast_node *expression=var_id->next;
            assert(var_id->type==id && is_expr(expression));

            symbol *var_sym = search_sym(var_id->value);
            if (!var_sym) {
                // symbol undeclared, do nothing
                err_node(sym_undef, node);
                return;
            } else if (var_sym->type != variable) {
                // wrong type, do nothing
                err_node(sym_undef, node);
                return;
            }

            gen_node(expression);

            // store result
            gen(STO, symtbl->layer-var_sym->layer, var_sym->value);
        }
        break;
        case if_stat: {
            assert(node->child);
            ast_node *cond=node->child;
            ast_node *body=cond->next;
            assert(cond->type==condition && is_statement(body));

            gen_node(cond);
            int jmp_code = code_loc;        // save code address
            gen(JPC, 0, 0);                 // temp jump target
            gen_node(body);
            code[jmp_code].a = code_loc;    // jump target backfill
        }
        break;
        case while_stat: {
            assert(node->child);
            ast_node *cond=node->child;
            ast_node *body=cond->next;
            assert(cond->type==condition && is_statement(body));

            int start_loc = code_loc;       // start of the statement
            gen_node(cond);
            int jmp_code = code_loc;        // save code address
            gen(JPC, 0, 0);                 // temp jump target
            gen_node(body);
            gen(JMP, 0, start_loc);         // loop
            code[jmp_code].a = code_loc;    // jump target backfill
        }
        break;
        case call_stat: {
            assert(node->child);
            ast_node *proc_id=node->child;
            ast_node *args=proc_id->next;
            assert(proc_id->type==id && args->type==arg_dec);

            symbol *proc_sym = search_sym(proc_id->value);
            if (!proc_sym) {
                // symbol undeclared, do nothing
                err_node(sym_undef, node);
                return;
            } else if (proc_sym->type != procedure) {
                // wrong type, do nothing
                err_node(sym_wrong_type, node);
                return;
            }

            // load arguments
            int arg_num=count_child(args);
            ast_node *arg=args->child;
            while (arg) {
                assert(is_expr(arg));
                gen_node(arg);
                arg = arg->next;
            }

            gen(CAL, symtbl->layer-proc_sym->layer, proc_sym->value);
            // unload arguments
            if (arg_num) {
                gen(INC, 0, -arg_num);
            }
        }
        break;
        case read_stat: {
            assert(node->child);
            ast_node *var_id=node->child;
            assert(var_id->type==id);

            symbol *var_sym = search_sym(var_id->value);
            if (!var_sym) {
                // symbol undeclared, do nothing
                err_node(sym_undef, node);
                return;
            } else if (var_sym->type != variable) {
                // wrong type, do nothing
                err_node(sym_undef, node);
                return;
            }

            gen(SI, 0, 0);
            gen(STO, symtbl->layer-var_sym->layer, var_sym->value);
        }
        break;
        case write_stat: {
            assert(node->child);
            ast_node *expression=node->child;
            assert(is_expr(expression));

            gen_node(expression);
            gen(SO, 0, 0);
        }
        break;
        case block_stat: {
            assert(node->child);
            ast_node *tmp=node->child;
            while (tmp) {
                assert(is_statement(tmp));
                gen_node(tmp);
                tmp = tmp->next;
            }
        }
        break;
        case condition: {
            assert(node->child);
            ast_node *lhs=node->child;
            ast_node *rhs=lhs->next;
            assert(is_expr(lhs) && is_expr(rhs));

            gen_node(lhs);
            gen_node(rhs);

            switch (node->value) {
                case EQU_OP: gen(OPR, 0, 8); break;
                case NEQ_OP: gen(OPR, 0, 9); break;
                case LSS_OP: gen(OPR, 0, 10); break;
                case LEQ_OP: gen(OPR, 0, 11); break;
                case GTR_OP: gen(OPR, 0, 12); break;
                case GEQ_OP: gen(OPR, 0, 13); break;
                default: exit(1);
            }
        }
        break;
        case expr: {
            assert(node->child);
            ast_node *lhs=node->child;
            ast_node *rhs=lhs->next;
            assert(is_expr(lhs) && is_expr(rhs));

            gen_node(lhs);
            gen_node(rhs);

            if (optimize && code[code_loc-1].f == LIT && code[code_loc-2].f == LIT) {
                // constant folding
                code_loc--;
                switch (node->value) {
                    case ADD_OP: code[code_loc-1].a += code[code_loc].a; break;
                    case SUB_OP: code[code_loc-1].a -= code[code_loc].a; break;
                    case MUL_OP: code[code_loc-1].a *= code[code_loc].a; break;
                    case DIV_OP: code[code_loc-1].a /= code[code_loc].a; break;
                    default: exit(1);
                }
                break;
            }

            switch (node->value) {
                case ADD_OP: gen(OPR, 0, 2); break;
                case SUB_OP: gen(OPR, 0, 3); break;
                case MUL_OP: gen(OPR, 0, 4); break;
                case DIV_OP: gen(OPR, 0, 5); break;
                default: exit(1);
            }
        }
        break;
        case num: {
            gen(LIT, 0, node->value);
        }
        break;
        case id: {
            symbol *sym = search_sym(node->value);
            if (!sym) {
                // symbol undeclared, push 0 instead
                gen(LIT, 0, 0);
                err_node(sym_undef, node);
            } else if (sym->type == constant) {
                // constant, push immediate number
                gen(LIT, 0, sym->value);
            } else if (sym->type == variable) {
                // variable, load from stack
                gen(LOD, symtbl->layer-sym->layer, sym->value);
            } else {
                // wrong type, push 0 instead
                gen(LIT, 0, 0);
                err_node(sym_wrong_type, node);
            }
        }
        break;

        default:
        break;
    }
}

void err_node(err_type type, ast_node *node) {
    switch (type) {
        case sym_undef: {
            fprintf(stderr, "%d:%d error: use of undeclared identifier %s\n", node->loc.first_line, node->loc.first_column, ident_table[node->value]);
        }
        break;

        case sym_redef: {
            fprintf(stderr, "%d:%d error: redeclaration of %s\n", node->loc.first_line, node->loc.first_column, ident_table[node->value]);
        }
        break;

        case sym_wrong_type: {
            fprintf(stderr, "%d:%d error: wrong type of symbol %s\n", node->loc.first_line, node->loc.first_column, ident_table[node->value]);
        }
        break;
    }
}
