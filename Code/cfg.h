/*
 * File: cfg.h
 * Project: C--_Compiler
 * File Created: 2020-05-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#ifndef _CFG_H
#define _CFG_H

#include "common.h"
#include "intercode.h"

#define MAX_BB 128
#define MAX_T 50000
#define MAX_V 10000

typedef struct _BasicBlock* BasicBlock_ptr;
typedef struct _BasicBlock {
	int id, is_start, is_finish, from_num;
	InterCodes start, finish;
	BasicBlock_ptr adj_to, jump_to, nxt;
	BasicBlock_ptr from[MAX_BB];
	// Const Reduction
	int *in_t, *in_v, *out_t, *out_v, *out_t_prev, *out_v_prev;
} BasicBlock;

typedef struct _Procedure {
	InterCodes func;
	BasicBlock_ptr bb, enter, exit;
	int num_bb;
} Procedure;

void build_procedures();
void reduce_constant();

//Debug
void log_cfg();

#endif

/*--------------------------------------------------------------------
 * cfg.h
 *------------------------------------------------------------------*/
