/**
 *
 * C-- Compiler, Labs of NJU course Principles and Techiques of Compiler
 *
 * Author: Zangwei Zheng, zhengzangw@163.com
 *
 **/
#include "common.h"
#include "error.h"
#include "ast.h"

int main(int argc, char **argv){
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
#ifdef DEBUG
    yydebug = 1;
#endif
    yyparse();
    if (error_num==0) ast_print(ast_root, 0);
    return 0;
}

/*--------------------------------------------------------------------
 * main.c
 *------------------------------------------------------------------*/