/*
 * File: main.c
 * Project: C--_Compiler
 * File Created: 2020-03-29
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "ast.h"
#include "cfg.h"
#include "common.h"
#include "error.h"
#include "intercode.h"
#include "optimize.h"
#include "semantic.h"
#include "symbol.h"
#include "syntax.h"
#include "translate.h"
#include "mips.h"

#ifndef Lab
#define Lab 4
#endif

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
#ifdef YYDEBUG
    yydebug = 1;
#endif

    yyparse();

    if (error_num == 0) {
#if Lab == 1
        ast_print(ast_root, 0);
#endif

#if Lab >= 2
        Program(ast_root);  // semantic errors
#ifdef DEBUG
        _hash_print_all_symbols();
#endif
#endif

#if Lab >= 3
        translate_Program(ast_root);
#if OP >= 2
        optimize();
#endif
        FILE* fp;
        if (argc <= 2) {
            fp = stdout;
        } else {
            fp = fopen(argv[2], "w");
        }
#endif

#if Lab == 3
        output_intercodes(intercodes_t, fp);
#endif

#if Lab == 4
        output_intercodes(intercodes_t, stdout);
        output_mips_instructions(intercodes_t, fp);
#endif

#if Lab >= 3
        fclose(fp);
#endif
    } else {
        print_error();  // lexical & semantic errors
    }

    return 0;
}

/*--------------------------------------------------------------------
 * main.c
 *------------------------------------------------------------------*/
