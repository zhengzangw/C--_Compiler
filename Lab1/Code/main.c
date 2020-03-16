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

    //yydebug = 1;

    yyparse();
    if (error_num==0) ast_print(ast_root, 0);
    else {
        print_error();
        //printf("Total error: %d\n", error_num);
    }
    return 0;
}

/*--------------------------------------------------------------------
 * main.c
 *------------------------------------------------------------------*/