/*
 * File: error.c
 * Project: C--_Compiler
 * File Created: 2020-02-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "error.h"
#include "common.h"

int error_num = 0;
int newline_error = 1;
char yyerror_s[256];
int throw_error(char type, int lineno, char* msg, char* detail) {
    if (newline_error) {
        error_num++;
        switch (type) {
            case 'A':
                sprintf(yyerror_s, "Error type A at Line %d: %s \"%s\".\n",
                        lineno, msg, detail);
                newline_error = 0;
                break;
            case 'B':
                sprintf(
                    yyerror_s,
                    "Error type B at Line %d: syntax error near \"%s\", %s.\n",
                    lineno, detail, msg);
                newline_error = 0;
                break;
            case 'F':  // redirected by yyerror
                sprintf(
                    yyerror_s,
                    "Error type B at Line %d: syntax error near \"%s\", %s.\n",
                    lineno, detail, msg);
                break;
        }
    }
    return 0;
}

int print_error() {
    if (strlen(yyerror_s) > 0) {
        fprintf(stderr, "%s", yyerror_s);
        yyerror_s[0] = '\0';
    }
    return 0;
}

/*--------------------------------------------------------------------
 * error.c
 *------------------------------------------------------------------*/
