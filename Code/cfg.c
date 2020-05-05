/*
 * File: cfg.c
 * Project: C--_Compiler
 * File Created: 2020-05-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "cfg.h"

Procedure funcs[128];
int num_funcs = 0;

void build_cfg(InterCodes, int);

void build_procedures() {
    InterCodes p = intercodes_t;
    while (p) {
        if (!p->code->disabled) {
            if (p->code->kind == IR_FUNC) {
                funcs[num_funcs++].func = p;
                p = p->prev;
                while (p->code->kind == IR_PARAM) p = p->prev;
                build_cfg(p, num_funcs - 1);
            }
        }
        p = p->prev;
    }
}

BasicBlock_ptr new_bb(InterCodes cur, InterCodes pre, int f_no) {
    BasicBlock_ptr bb = (BasicBlock_ptr)malloc(sizeof(BasicBlock));
    bb->start = cur;
    bb->finish = NULL;
    bb->jump_to = NULL;
    bb->is_start = bb->is_finish = 0;
    if (funcs[f_no].num_bb == 0) {
        funcs[f_no].bb = bb;
        funcs[f_no].enter = bb;
    } else {
        funcs[f_no].bb->finish = pre;
        funcs[f_no].bb->adj_to = bb;
        bb->nxt = funcs[f_no].bb;
        funcs[f_no].bb = bb;
    }
    bb->id = funcs[f_no].num_bb++;
    return bb;
}

BasicBlock_ptr bb_at(int id, int f_no) {
    BasicBlock_ptr p = funcs[f_no].enter;
    while (p && p->id != id) p = p->adj_to;
    return p;
}

void build_cfg(InterCodes st, int f_no) {
    int *label_belong = (int *)calloc(label_num, sizeof(int));
    // Build Enter Block
    BasicBlock_ptr enter = new_bb(NULL, NULL, f_no);
    enter->is_start = 1;
    // Build Blocks
    InterCodes p = st;
    InterCodes pre = NULL;
    while (p && p->code->kind != IR_FUNC) {
        if (!p->code->disabled) {
            if (!pre || pre->code->kind == IR_GOTO ||
                pre->code->kind == IR_RELOP || pre->code->kind == IR_RET)
                new_bb(p, pre, f_no);
            if (p->code->kind == IR_LABEL) {
                label_belong[p->code->x->u.label_no] = funcs[f_no].num_bb - 1;
            }
            pre = p;
        }
        p = p->prev;
    }
    // Build Exit Block
    BasicBlock_ptr exit = new_bb(p, pre, f_no);
    exit->is_finish = 1;
    // Link Blocks
    BasicBlock_ptr p_b = enter;
    while (p_b) {
        if (p_b->finish) {
            switch (p_b->finish->code->kind) {
                case IR_GOTO:
                    p_b->jump_to = bb_at(
                        label_belong[p_b->finish->code->x->u.label_no], f_no);
                    break;
                case IR_RELOP:
                    p_b->jump_to = bb_at(
                        label_belong[p_b->finish->code->z->u.label_no], f_no);
                    break;
                case IR_RET:
                    p_b->jump_to =
                        bb_at(label_belong[funcs[f_no].num_bb - 1], f_no);
                    break;
                default:
                    break;
            }
        }
        p_b = p_b->adj_to;
    }
}

void log_cfg() {
    for (int i = 0; i < num_funcs; ++i) {
        printf(">>> %s <<<\n", funcs[i].func->code->x->u.val);
        BasicBlock_ptr p = funcs[i].enter;
        while (p) {
            if (p->is_start)
                printf("ENTER(%d)\n", p->id);
            else if (p->is_finish)
                printf("FINISH(%d)\n", p->id);
            else {
                printf(">>> Block(%d) adj:%d ", p->id, p->adj_to->id);
                if (p->jump_to) printf("goto:%d", p->jump_to->id);
                printf("<<<\n");

                InterCodes ir = p->start;
                while (ir && ir != p->finish) {
                    if (!ir->code->disabled) {
                        output_intercode(ir->code, stdout);
                    }
                    ir = ir->prev;
                }
                if (p->finish) output_intercode(p->finish->code, stdout);
                printf("=======\n");
            }
            p = p->adj_to;
        }
        printf("============\n");
    }
}

/*--------------------------------------------------------------------
 * cfg.c
 *------------------------------------------------------------------*/
