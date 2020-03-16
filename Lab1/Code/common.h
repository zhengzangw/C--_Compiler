/**
 *
 * C-- Compiler, Labs of NJU course Principles and Techiques of Compiler
 *
 * Author: Zangwei Zheng, zhengzangw@163.com
 *
 **/

#ifndef _COMMON_H
#define _COMMON_H

/*--------------------------------------------------------------------
 *
 * define
 *
 *------------------------------------------------------------------*/
#define LOG(x) printf("\e[32m%s\e[0m\n", x)
#define LOGINT(x) printf("\e[32m%d\e[0m\n", x)
#define LOGFLOAT(x) printf("\e[32m%s\e[0m\n", x)
#define OUT(x) printf("\e[31m%s\e[0m\n", x)
#define OUTINT(x) printf("\e[31m%d\e[0m\n", x)
#define OUTFLOAT(x) printf("\e[31m%s\e[0m\n", x)

//#define YYDEBUG 1
//extern int yydebug;

/*--------------------------------------------------------------------
 *
 * include
 *
 *------------------------------------------------------------------*/
#include <stdio.h>

/*--------------------------------------------------------------------
 *
 * Flex & Bison
 *
 *------------------------------------------------------------------*/
void yyrestart (FILE*);
int yyparse (void);
int yyerror(char*);

#endif

/*--------------------------------------------------------------------
 * common.h
 *------------------------------------------------------------------*/