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
char yyerror_s[128];
int throw_error(char type, int lineno, char* msg, char* detail){
    if (newline_error){
        error_num++;
        switch (type){
            case 'A':
                sprintf(yyerror_s, "Error type A at Line %d: %s \"%s\"\n",lineno, msg, detail);
                newline_error = 0;
                break;
            case 'B':
                sprintf(yyerror_s, "Error type B at Line %d: syntax error near \"%s\", %s\n", lineno, detail, msg);
                newline_error = 0;
                break;
            case 'F': //redirected by yyerror
                sprintf(yyerror_s, "Error type B at Line %d: syntax error near \"%s\", %s\n", lineno, detail, msg);
                break;
        }
    }
    return 0;
}

int print_error(){
    fprintf(stderr, "%s", yyerror_s);
}

/*--------------------------------------------------------------------
 * error.c
 *------------------------------------------------------------------*/