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

// Assist
#define LOG(x) printf("\e[32m%s\e[0m\n", x)
#define LOGINT(x) printf("\e[32m%d\e[0m\n", x)
#define LOGFLOAT(x) printf("\e[32m%s\e[0m\n", x)
#define OUT(x) printf("\e[31m%s\e[0m\n", x)
#define OUTINT(x) printf("\e[31m%d\e[0m\n", x)
#define OUTFLOAT(x) printf("\e[31m%s\e[0m\n", x)
#define true 1
#define false 0

// Optimization
#define OP 3
#if OP >= 3
	#define OP_CFG_CONST
	#define OP_CFG_DEAD_CODE
#endif
#if OP >= 2
	#define OP_LINEAR_USELESS_LABEL
	#define OP_LINEAR_REPLICATE_LABEL
	#define OP_LINEAR_DIRECT_GOTO
	#define OP_LINEAR_REDUCE_RELOP
#endif
#if OP >= 1
	#define OP_INT
	#define OP_ID
	#define OP_TEMP_REPLACE
	#define OP_ARITH_CONST
	#define OP_ARR_CONST
	#define OP_ASSIGN_TO_VAR
#endif

// Debug
// #define DEBUG
// #define YYDEBUG 1
#ifdef YYDEBUG
	extern int yydebug;
#endif

/*--------------------------------------------------------------------
 *
 * include
 *
 *------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#endif

/*--------------------------------------------------------------------
 * common.h
 *------------------------------------------------------------------*/
