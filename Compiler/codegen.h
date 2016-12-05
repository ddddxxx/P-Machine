#ifndef codegen_h
#define codegen_h

#include <stdbool.h>
#include "ast.h"

#define SYM_TABLE_INIT_SIZE 100
#define CODE_INIT_SIZE 100

typedef enum mnemonic mnemonic;
typedef struct instruction instruction;
typedef enum symbol_type symbol_type;
typedef struct symbol symbol;
typedef struct symbol_table symbol_table;

enum mnemonic {
    LIT,    OPR,    LOD,    STO,
    CAL,    RET,    INC,    JMP,
    JPC,    SI,     SO,
};
struct instruction {
    mnemonic f;
    int l;
    int a;
};

enum symbol_type {
    constant,
    variable,
    procedure,
};
struct symbol {
    symbol_type type;
    int name;       // index of ident table
    int value;      // value for const, addr for var and proc
    int layer;      // for var and proc
};
struct symbol_table {
    symbol *syms;
    int num;                // syms number
    int size;               // syms size
    int layer;
    int size_to_alloc;      // proc alloc size
    symbol_table *outer;    // outer proc symbol table
};

extern bool optimize;

extern instruction *code;
extern int code_loc;
extern int code_size;
void gen(mnemonic f, int l, int a);

void print_code();
void print_raw_code();

extern symbol_table *symtbl;
symbol *creat_sym(symbol_type type, int name, int value);
void creat_sym_table();
void deatroy_current_sym_table();

symbol *search_sym(int name);

void gen_node(ast_node *node);


typedef enum err_type {
    sym_undef,
    sym_redef,
    sym_wrong_type,
} err_type;

void err_node(err_type type, ast_node *node);

#endif
