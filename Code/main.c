/*
 * File: main.c
 * Project: C--_Compiler
 * File Created: 2020-03-29
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "ast.h"
#include "common.h"
#include "error.h"
#include "intercode.h"
#include "semantic.h"
#include "symbol.h"
#include "translate.h"
#include "optimize.h"
#include "cfg.h"

#define Lab3

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);

    // yydebug = 1;

    yyparse();

    if (error_num == 0) {
#ifdef Lab1
        ast_print(ast_root, 0);
#endif
#ifdef Lab2
        Program(ast_root);
        // _hash_print_all_symbols();
#endif
#ifdef Lab3
        Program(ast_root);
        translate_Program(ast_root);
#ifdef OP1
		optimize();
#endif
        FILE* fp;
        if (argc <= 2) {
            fp = stdout;
        } else {
            fp = fopen(argv[2], "w");
        }
        output_intercodes(intercodes_t, fp);
        fclose(fp);
#endif
    } else {
        print_error();
    }

    return 0;
}

/*--------------------------------------------------------------------
 * main.c
 *------------------------------------------------------------------*/
