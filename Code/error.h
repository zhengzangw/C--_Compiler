/**
 *
 * C-- Compiler, Labs of NJU course Principles and Techiques of Compiler
 *
 * Author: Zangwei Zheng, zhengzangw@163.com
 *
 **/
#ifndef _ERROR_H
#define _ERROR_H

#define errorA(msg) throw_error('A', yylineno, msg, yytext)
#define errorB(msg) throw_error('B', yylineno, msg, yylval.node->original_val)
#define errorF() throw_error('F', yylineno, "be careful", yylval.node->original_val)

int throw_error(char type, int lineno, char* msg, char* detail);
int throw_fallback_error();

extern int error_num;
extern int newline_error;
extern int fallback_error;

#endif

/*--------------------------------------------------------------------
 * error.h
 *------------------------------------------------------------------*/