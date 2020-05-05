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

typedef struct _BasicBlock* BasicBlock_ptr;
typedef struct _BasicBlockListNode {
	BasicBlock_ptr node, nxt;
} BasicBlockListNode;
typedef struct _BasicBlock {
	int id, is_start, is_finish;
	InterCodes start, finish;
	BasicBlock_ptr adj_to, jump_to, nxt;
	BasicBlockListNode from;
} BasicBlock;

typedef struct _Procedure {
	InterCodes func;
	BasicBlock_ptr bb, enter;
	int num_bb;
} Procedure;

void build_procedures();

//Debug
void log_cfg();

#endif

/*--------------------------------------------------------------------
 * cfg.h
 *------------------------------------------------------------------*/
