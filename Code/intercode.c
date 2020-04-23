/*
 * File: intercode.c
 * Project: C--_Compiler
 * File Created: 2020-04-21
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "intercode.h"

#include "common.h"

InterCodes intercodes_s = NULL;
InterCodes intercodes_t = NULL;

void insert_intercode(InterCode ir) {
    InterCodes tmp_code = (InterCodes)malloc(sizeof(struct InterCodes_));
    tmp_code->code = ir;
    if (intercodes_s) {
        tmp_code->next = intercodes_s;
        tmp_code->prev = NULL;
        intercodes_s->prev = tmp_code;
        intercodes_s = tmp_code;
    } else {
        intercodes_s = intercodes_t = tmp_code;
        tmp_code->next = tmp_code->prev = NULL;
    }
}

void output_op(Operand op, FILE *fp) {}

void output_intercode(FILE *fp) {
    InterCodes p = intercodes_t;
    while (p) {
        p = p->prev;
    }
}

/*--------------------------------------------------------------------
 * intercode.c
 *------------------------------------------------------------------*/
