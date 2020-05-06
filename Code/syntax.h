/*
 * File: lexical.h
 * Project: Code
 * File Created: 2020-03-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "common.h"

void yyrestart(FILE*);
int yyparse(void);
int yyerror(char*);

/*--------------------------------------------------------------------
 * syntax.h
 *------------------------------------------------------------------*/
