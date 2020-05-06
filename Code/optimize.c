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
    InterCodes p;
    int *label_h = (int *)calloc(label_num, sizeof(int));
    int *label_comb = (int *)calloc(label_num, sizeof(int));
    // Linear 1
    p = intercodes_t;
    while (p) {
        if (!p->code->disabled) {
#ifdef OP_LINEAR_REDUCE_RELOP
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
#endif
#ifdef OP_LINEAR_USELESS_LABEL
            // Delete useless label
            if (p->code->kind == IR_RELOP) {
                label_h[p->code->z->u.label_no]++;
            }
            if (p->code->kind == IR_GOTO) {
                label_h[p->code->x->u.label_no]++;
            }
#endif
        }
        p = p->prev;
    }
    // Linear 2
    p = intercodes_t;
    InterCodes pre = NULL;
    while (p) {
        if (!p->code->disabled) {
#ifdef OP_LINEAR_USELESS_LABEL
            // Delete useless label
            if (p->code->kind == IR_LABEL && !label_h[p->code->x->u.label_no])
                p->code->disabled = 1;
#endif
#ifdef OP_LINEAR_REPLICATE_LABEL
            // Combine labels
            if (!p->code->disabled && p->code->kind == OP_LABEL && pre &&
                pre->code->kind == OP_LABEL) {
                if (label_comb[pre->code->x->u.label_no])
                    label_comb[p->code->x->u.label_no] =
                        label_comb[pre->code->x->u.label_no];
                else
                    label_comb[p->code->x->u.label_no] =
                        pre->code->x->u.label_no;
            }
            if (!p->code->disabled) pre = p;
#endif
        }
        p = p->prev;
    }
    // Linear 3
#ifdef OP_LINEAR_REPLICATE_LABEL
    p = intercodes_t;
    while (p) {
        if (!p->code->disabled) {
            // Combine labels
            if (p->code->kind == IR_LABEL &&
                label_comb[p->code->x->u.label_no]) {
                p->code->disabled = 1;
            } else if (p->code->kind == IR_GOTO &&
                       label_comb[p->code->x->u.label_no]) {
                int l = p->code->x->u.label_no;
                p->code->x = new_label();
                p->code->x->u.label_no = label_comb[l];
            } else if (p->code->kind == IR_RELOP &&
                       label_comb[p->code->z->u.label_no]) {
                int l = p->code->z->u.label_no;
                p->code->z = new_label();
                p->code->z->u.label_no = label_comb[l];
            }
        }
        p = p->prev;
    }
#endif
#ifdef OP_LINEAR_DIRECT_GOTO
    // Linear 4
    p = intercodes_t;
    while (p) {
        if (!p->code->disabled) {
            if (p->code->kind == IR_RELOP && p->code->x->kind == OP_CONSTANT &&
                p->code->y->kind == OP_CONSTANT) {
                int a = p->code->x->u.value;
                int b = p->code->y->u.value;
                int flag;
                if (strcmp(p->code->relop, "==") == 0) {
                    flag = (a == b);
                } else if (strcmp(p->code->relop, "!=") == 0) {
                    flag = (a != b);
                } else if (strcmp(p->code->relop, "<") == 0) {
                    flag = (a < b);
                } else if (strcmp(p->code->relop, ">") == 0) {
                    flag = (a > b);
                } else if (strcmp(p->code->relop, ">=") == 0) {
                    flag = (a >= b);
                } else if (strcmp(p->code->relop, "<=") == 0) {
                    flag = (a <= b);
                }
                if (flag) {
                    p->code->kind = IR_GOTO;
                    *p->code->x = *p->code->z;
                } else {
                    p->code->disabled = 1;
                }
            }
        }
        p = p->prev;
    }
#endif
#ifdef OP_CFG_CONST
    build_procedures();
    reduce_constant();
#endif
#ifdef OP_CFG_DEAD_CODE
    remove_dead_code();
#endif
}

/*--------------------------------------------------------------------
 * optimize.h
 *------------------------------------------------------------------*/
