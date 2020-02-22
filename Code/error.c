/**
 *
 * C-- Compiler, Labs of NJU course Principles and Techiques of Compiler
 *
 * Author: Zangwei Zheng, zhengzangw@163.com
 *
 **/

#include "common.h"
#include "error.h"

int error_num = 0;
int newline_error = 1;
int throw_error(char type, int lineno, char* msg, char* detail){
    if (newline_error){
        error_num++;
        switch (type){
            case 'A':
                fprintf(stderr, "Error type A at Line %d: %s \"%s\"\n",lineno, msg, detail);
                break;
            case 'B':
                fprintf(stderr, "Error type B at Line %d: syntax error near \"%s\", %s\n", lineno, detail, msg);
                break;
        }
        newline_error = 0;
    }
    return 0;
}

/*--------------------------------------------------------------------
 * error.c
 *------------------------------------------------------------------*/