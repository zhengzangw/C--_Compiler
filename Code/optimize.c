/*
 * File: optimize.c
 * Project: C--_Compiler
 * File Created: 2020-05-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "optimize.h"

#include "cfg.h"
#include "intercode.h"

void optimize() {
    // Linear
    InterCodes p = intercodes_t;
    int *label_h = (int *)calloc(label_num, sizeof(int));
    while (p) {
        if (!p->code->disabled) {
            // IF_RELOP case 1
            if (p->code->kind == IR_RELOP && p->prev &&
                p->prev->code->kind == IR_GOTO && p->prev->prev &&
                p->prev->prev->code->kind == IR_LABEL &&
                p->code->z->u.label_no == p->prev->prev->code->x->u.label_no) {
                p->code->z = new_label();
                p->code->z->u.label_no = p->prev->code->x->u.label_no;
                p->prev->code->disabled = 1;
                if (strcmp(p->code->relop, "==") == 0) {
                    strcpy(p->code->relop, "!=");
                } else if (strcmp(p->code->relop, "!=") == 0) {
                    strcpy(p->code->relop, "==");
                } else if (strcmp(p->code->relop, "<") == 0) {
                    strcpy(p->code->relop, ">=");
                } else if (strcmp(p->code->relop, ">") == 0) {
                    strcpy(p->code->relop, "<=");
                } else if (strcmp(p->code->relop, ">=") == 0) {
                    strcpy(p->code->relop, "<");
                } else if (strcmp(p->code->relop, "<=") == 0) {
                    strcpy(p->code->relop, ">");
                }
            }
            // IF_RELOP case 2
            if (p->code->kind == IR_RELOP && p->prev &&
                p->prev->code->kind == IR_GOTO && p->prev->prev &&
                p->prev->prev->code->kind == IR_LABEL &&
                p->prev->code->x->u.label_no ==
                    p->prev->prev->code->x->u.label_no) {
                p->prev->code->disabled = 1;
            }
            // Delete useless label
            if (p->code->kind == IR_RELOP) {
                label_h[p->code->z->u.label_no]++;
            }
            if (p->code->kind == IR_GOTO) {
                label_h[p->code->x->u.label_no]++;
            }
        }
        p = p->prev;
    }
    // CFG
    build_procedures();
    // Linear
    p = intercodes_t;
    while (p) {
        // Delete useless label
        if (p->code->kind == IR_LABEL && !label_h[p->code->x->u.label_no]) {
            p->code->disabled = 1;
        }
        p = p->prev;
    }
}

/*--------------------------------------------------------------------
 * optimize.h
 *------------------------------------------------------------------*/
