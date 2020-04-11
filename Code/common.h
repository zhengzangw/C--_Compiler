/*
 * File: common.h
 * Project: C--_Compiler
 * File Created: 2020-02-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

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
#define true 1
#define false 0

//#define YYDEBUG 1
// extern int yydebug;

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
void yyrestart(FILE*);
int yyparse(void);
int yyerror(char*);

#endif

/*--------------------------------------------------------------------
 * common.h
 *------------------------------------------------------------------*/
