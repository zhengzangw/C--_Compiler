/*
 * File: error.h
 * Project: C--_Compiler
 * File Created: 2020-02-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#ifndef _ERROR_H
#define _ERROR_H

#define errorA(msg) throw_error('A', yylineno, msg, yytext)
#define errorB(msg) throw_error('B', yylineno, msg, yylval.node->original_val)
#define errorF() \
    throw_error('F', yylineno, "be careful", yylval.node->original_val)

int throw_error(char type, int lineno, char* msg, char* detail);
int print_error();

extern int error_num;
extern int newline_error;

#endif

/*--------------------------------------------------------------------
 * error.h
 *------------------------------------------------------------------*/
