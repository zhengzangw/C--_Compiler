/*
 * File: main.c
 * Project: C--_Compiler
 * File Created: 2020-03-29
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "ast.h"
#include "common.h"
#include "error.h"
#include "semantic.h"
#include "symbol.h"

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
        // Lab 1
        // ast_print(ast_root, 0);
        // Lab 2
        hash_create();
        Program(ast_root);
        //_hash_print_all_symbols();
    } else {
        print_error();
    }

    return 0;
}

/*--------------------------------------------------------------------
 * main.c
 *------------------------------------------------------------------*/
