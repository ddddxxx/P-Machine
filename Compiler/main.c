#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include "syntax.tab.h"
#include "ast.h"
#include "codegen.h"
#include "interpreter.h"

#define FILE_NAME_SIZE_MAX 256

extern FILE * yyin;
extern void yyparse();

static const char *optString = "tcroh";
bool out_ast_file = false;
bool out_code_file = false;
bool out_raw_file = false;
bool optimize = false;
char input_file_name[FILE_NAME_SIZE_MAX];
char ast_file_name[FILE_NAME_SIZE_MAX];
char pcode_file_name[FILE_NAME_SIZE_MAX];
char raw_file_name[FILE_NAME_SIZE_MAX];
FILE *ast_out = NULL;
FILE *code_out = NULL;
FILE *raw_out = NULL;

char *usage = "\
usage: compiler [options...] [filename]\n\
    -t  generate abstract syntax tree file\n\
    -c  generate pcode file\n\
    -r  generate hex file for Logisim\n\
    -o  optimize code\n\
\n\
With no FILE, read standard input.\
";

// static const struct option long_opts[] = {
//     { "ast",        no_argument,    NULL,   't' },
//     { "pcode",      no_argument,    NULL,   'c' },
//     { "hex",        no_argument,    NULL,   'r' },
//     { "optimize",   no_argument,    NULL,   'o' },
//     { "help",       no_argument,    NULL,   'h' },
//     { NULL,         0,              NULL,   0   }
// };

int main(int argc, char *argv[]) {
    ast_out=code_out=stdout;

    int opt = 0;
    while ((opt = getopt(argc, argv, optString)) != -1) {
        switch (opt) {
            case 't':
                out_ast_file = true;
                break;
            case 'c':
                out_code_file = true;
                break;
            case 'r':
                out_raw_file = true;
                break;
            case 'o':
                optimize = true;
                break;
            case 'h':
                puts(usage);
                return 0;
        }
    }
    if (optind<argc) {
        strcpy(input_file_name, argv[optind]);
        char *pdot = strrchr(input_file_name, '.');
        if (strcmp(pdot, ".pl0")) {
            puts("please input .pl0 file\n");
            exit(2);
        }
        if (!(yyin = fopen(input_file_name, "r"))) {
            puts("cannot open input file\n");
            exit(3);
        }
    } else {
        yyin = stdin;
    }

    yyparse();
    if (out_ast_file) {
        strcpy(ast_file_name, input_file_name);
        char *pdot = strrchr(ast_file_name, '.');
        strcpy(pdot, ".ast");
        if ((ast_out = fopen(ast_file_name, "w"))) {
            traverse(root, 0);
            fclose(ast_out);
        } else {
            puts("cannot write AST file\n");
        }
    }

    gen_node(root);
    if (out_code_file) {
        strcpy(pcode_file_name, input_file_name);
        char *pdot = strrchr(pcode_file_name, '.');
        strcpy(pdot, ".pcode");
        if ((code_out = fopen(pcode_file_name, "w"))) {
            print_code();
            fclose(code_out);
        } else {
            puts("cannot write pcode file\n");
        }
    }
    if (out_raw_file) {
        strcpy(raw_file_name, input_file_name);
        char *pdot = strrchr(raw_file_name, '.');
        strcpy(pdot, ".hex");
        if ((raw_out = fopen(raw_file_name, "w"))) {
            print_raw_code();
            fclose(raw_out);
        } else {
            puts("cannot write raw file\n");
        }
    }

    interpret(code);

    return 0;
}
