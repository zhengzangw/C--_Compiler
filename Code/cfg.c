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

#include "symbol.h"

Procedure funcs[128];
int num_funcs = 0;
int const_change = 1;

#define UNDEF -2847201
#define NAC -17182277

void build_cfg(InterCodes, int);

void build_procedures() {
    temp_num++;
    var_num++;
    InterCodes p = intercodes_t;
    while (p) {
        if (!p->code->disabled) {
            if (p->code->kind == IR_FUNC) {
                funcs[num_funcs++].func = p;
                p = p->prev;
                InterCodes param = p;
                while (p->code->kind == IR_PARAM) p = p->prev;
                build_cfg(p, num_funcs - 1);
                while (param->code->kind == IR_PARAM) {
                    funcs[num_funcs - 1]
                        .enter->in_v[param->code->x->u.variable->cnt] = NAC;
                    param = param->prev;
                }
            }
        }
        p = p->prev;
    }
}

void init_bb_latice(BasicBlock_ptr bb) {
    bb->in_t = (int *)malloc(temp_num * sizeof(int));
    bb->in_v = (int *)malloc(var_num * sizeof(int));
    bb->out_t = (int *)malloc(temp_num * sizeof(int));
    bb->out_v = (int *)malloc(var_num * sizeof(int));
    bb->out_t_prev = (int *)malloc(temp_num * sizeof(int));
    bb->out_v_prev = (int *)malloc(var_num * sizeof(int));
    for (int i = 0; i < temp_num; ++i) {
        bb->in_t[i] = bb->out_t[i] = UNDEF;
    }
    for (int i = 0; i < temp_num; ++i) {
        bb->in_v[i] = bb->out_v[i] = UNDEF;
    }
    memcpy(bb->out_t_prev, bb->out_t, temp_num * sizeof(int));
    memcpy(bb->out_v_prev, bb->out_v, var_num * sizeof(int));
}

BasicBlock_ptr new_bb(InterCodes cur, InterCodes pre, int f_no) {
    BasicBlock_ptr bb = (BasicBlock_ptr)malloc(sizeof(BasicBlock));
    bb->start = cur;
    bb->finish = NULL;
    bb->jump_to = NULL;
    bb->is_start = bb->is_finish = 0;
    bb->from_num = 0;
    init_bb_latice(bb);
    if (funcs[f_no].num_bb == 0) {
        funcs[f_no].bb = bb;
        funcs[f_no].enter = bb;
    } else {
        funcs[f_no].bb->finish = pre;
        funcs[f_no].bb->adj_to = bb;
        bb->nxt = funcs[f_no].bb;
        bb->from[bb->from_num++] = funcs[f_no].bb;
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
            else if (p->code->kind == IR_LABEL)
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
    funcs[f_no].exit = exit;
    // Link Blocks
    BasicBlock_ptr p_b = enter;
    while (p_b) {
        if (p_b->finish) {
            BasicBlock_ptr t;
            switch (p_b->finish->code->kind) {
                case IR_GOTO:
                    t = bb_at(label_belong[p_b->finish->code->x->u.label_no],
                              f_no);
                    p_b->jump_to = t;
                    t->from[t->from_num++] = p_b;
                    break;
                case IR_RELOP:
                    t = bb_at(label_belong[p_b->finish->code->z->u.label_no],
                              f_no);
                    p_b->jump_to = t;
                    t->from[t->from_num++] = p_b;
                    break;
                case IR_RET:
                    t = bb_at(funcs[f_no].num_bb - 1, f_no);
                    p_b->jump_to = t;
                    t->from[t->from_num++] = p_b;
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
        printf("### %s ###\n", funcs[i].func->code->x->u.val);
        BasicBlock_ptr p = funcs[i].enter;
        while (p) {
            if (p->is_start)
                printf(">>> ENTER(%d) ", p->id);
            else if (p->is_finish)
                printf(">>> FINISH(%d) ", p->id);
            else {
                printf(">>> Block(%d) adj:%d ", p->id, p->adj_to->id);
            }
            if (p->jump_to) printf("goto:%d ", p->jump_to->id);
            if (p->from_num) {
                printf("from:");
                for (int i = 0; i < p->from_num; ++i)
                    printf("%d ", p->from[i]->id);
            }
            printf("<<<\n");

            // Output Code
            if (!p->is_finish) {
                InterCodes ir = p->start;
                while (ir && ir != p->finish) {
                    if (!ir->code->disabled) {
                        output_intercode(ir->code, stdout);
                    }
                    ir = ir->prev;
                }
                if (p->finish) output_intercode(p->finish->code, stdout);
            }
            // Output OUT
            printf("OUT: ");
            for (int i = 0; i < temp_num; ++i) {
                if (p->out_t[i] != NAC && p->out_t[i] != UNDEF)
                    printf("t%d=%d ", i, p->out_t[i]);
                // if (p->out_t[i] == NAC) printf("t%d=NAC ", i);
            }
            for (int i = 0; i < var_num; ++i) {
                if (p->out_v[i] != NAC && p->out_v[i] != UNDEF)
                    printf("v%d=%d ", i, p->out_v[i]);
                // if (p->out_v[i] == NAC) printf("v%d=NAC ", i);
            }
            printf("\n");

            printf("=======\n");

            p = p->adj_to;
        }
        printf("#############\n");
    }
}

int const_val(Operand op, BasicBlock_ptr bb) {
    if (op->kind == OP_CONSTANT) return op->u.value;
    if (op->kind == OP_TEMP) return bb->out_t[op->u.tmp_no];
    if (op->kind == OP_VARIABLE) return bb->out_v[op->u.variable->cnt];
    return NAC;
}

void const_assign(Operand x, int val, BasicBlock_ptr bb) {
    if (x->kind == OP_TEMP && bb->out_t[x->u.tmp_no] != NAC) {
        if (bb->out_t[x->u.tmp_no] == UNDEF || bb->out_t[x->u.tmp_no] == val)
            bb->out_t[x->u.tmp_no] = val;
        else
            bb->out_t[x->u.tmp_no] = NAC;
    } else if (x->kind == OP_VARIABLE && bb->out_v[x->u.variable->cnt] != NAC) {
        if (bb->out_v[x->u.variable->cnt] == UNDEF ||
            bb->out_v[x->u.variable->cnt] == val)
            bb->out_v[x->u.variable->cnt] = val;
        else
            bb->out_v[x->u.variable->cnt] = NAC;
    }
}

void is_changed(BasicBlock_ptr bb) {
    for (int i = 0; i < temp_num; ++i) {
        if (bb->out_t_prev[i] != bb->out_t[i]) {
            const_change = 1;
            // printf("t%d=%d!=%d\n", i, bb->out_t[i], bb->out_t_prev[i]);
        }
    }
    for (int i = 0; i < var_num; ++i) {
        if (bb->out_v_prev[i] != bb->out_v[i]) {
            const_change = 1;
            // printf("v%d\n", i);
        }
    }
}

void compute_gen_kill_bb(BasicBlock_ptr bb) {
    InterCodes p = bb->start;
    int a, b;
    InterCodes pre = NULL;
    while (p && (!pre || pre != bb->finish)) {
        if (!p->code->disabled) {
            switch (p->code->kind) {
                case IR_ASSIGN:
                    const_assign(p->code->x, const_val(p->code->y, bb), bb);
                    break;
                case IR_ADD:
                    a = const_val(p->code->y, bb);
                    b = const_val(p->code->z, bb);
                    if (a != UNDEF && b != UNDEF && a != NAC && b != NAC)
                        const_assign(p->code->x, a + b, bb);
                    else if (a == NAC || b == NAC)
                        const_assign(p->code->x, NAC, bb);
                    else
                        const_assign(p->code->x, UNDEF, bb);
                    break;
                case IR_SUB:
                    a = const_val(p->code->y, bb);
                    b = const_val(p->code->z, bb);
                    if (a != UNDEF && b != UNDEF && a != NAC && b != NAC)
                        const_assign(p->code->x, a - b, bb);
                    else if (a == NAC || b == NAC)
                        const_assign(p->code->x, NAC, bb);
                    else
                        const_assign(p->code->x, UNDEF, bb);
                    break;
                case IR_MUL:
                    a = const_val(p->code->y, bb);
                    b = const_val(p->code->z, bb);
                    if (a != UNDEF && b != UNDEF && a != NAC && b != NAC)
                        const_assign(p->code->x, a * b, bb);
                    else if (a == NAC || b == NAC)
                        const_assign(p->code->x, NAC, bb);
                    else
                        const_assign(p->code->x, UNDEF, bb);
                    break;
                case IR_DIV:
                    a = const_val(p->code->y, bb);
                    b = const_val(p->code->z, bb);
                    if (a != UNDEF && b != UNDEF && a != NAC && b != NAC)
                        const_assign(p->code->x, a / b, bb);
                    else if (a == NAC || b == NAC)
                        const_assign(p->code->x, NAC, bb);
                    else
                        const_assign(p->code->x, UNDEF, bb);
                    break;
                case IR_GET_ADDR:
                case IR_GET_VAL:
                case IR_CALL:
                case IR_READ:
                    const_assign(p->code->x, NAC, bb);
                    break;
                default:
                    // do nothing
                    break;
            }
            pre = p;
        }
        p = p->prev;
    }
}

void update_bb(BasicBlock_ptr bb) {
    // update bb IN
    if (!bb->is_start) {
        for (int i = 0; i < bb->from_num; i++) {
            BasicBlock_ptr pre_bb = bb->from[i];
            if (i == 0) {
                memcpy(bb->in_t, pre_bb->out_t, temp_num * sizeof(int));
                memcpy(bb->in_v, pre_bb->out_v, var_num * sizeof(int));
            } else {
                for (int j = 0; j < temp_num; ++j) {
                    if (pre_bb->out_t[j] != UNDEF &&
                        pre_bb->out_t[j] != bb->in_t[j])
                        bb->in_t[j] = NAC;
                }
                for (int j = 0; j < var_num; ++j) {
                    if (pre_bb->out_v[j] != UNDEF &&
                        pre_bb->out_v[j] != bb->in_v[j])
                        bb->in_v[j] = NAC;
                }
            }
        }
    }
    // Log
    /*
printf("IN[%d] ", bb->id);
for (int i = 0; i < temp_num; ++i) {
    if (bb->in_t[i] != NAC && bb->in_t[i] != UNDEF) printf("t%d ", i);
    if (bb->in_t[i] == NAC) printf("t%d=NAC ", i);
}
for (int i = 0; i < var_num; ++i) {
    if (bb->in_v[i] != NAC && bb->in_v[i] != UNDEF) printf("v%d ", i);
    if (bb->in_v[i] == NAC) printf("v%d=NAC ", i);
}
if (bb->is_finish) printf("finish");
printf("\n");
    */
    // update bb OUT
    memcpy(bb->out_t_prev, bb->out_t, temp_num * sizeof(int));
    memcpy(bb->out_v_prev, bb->out_v, var_num * sizeof(int));
    memcpy(bb->out_t, bb->in_t, temp_num * sizeof(int));
    memcpy(bb->out_v, bb->in_v, var_num * sizeof(int));
    if (!bb->is_finish) {
        compute_gen_kill_bb(bb);
    }

    // Log
    /*
printf("OUT[%d] ", bb->id);
for (int i = 0; i < temp_num; ++i) {
    if (bb->out_t[i] != NAC && bb->out_t[i] != UNDEF) printf("t%d ", i);
}
for (int i = 0; i < var_num; ++i) {
    if (bb->out_v[i] != NAC && bb->out_v[i] != UNDEF) printf("v%d ", i);
}
if (bb->is_finish) printf("finish");
printf("\n");
    */
}

int check_op_const(Operand x, BasicBlock_ptr bb) {
    if (!x) return 0;
    if (x->kind == OP_TEMP && bb->out_t[x->u.tmp_no] != UNDEF &&
        bb->out_t[x->u.tmp_no] != NAC) {
        x->kind = OP_CONSTANT;
        x->u.value = bb->out_t[x->u.tmp_no];
        return 1;
    }
    if (x->kind == OP_VARIABLE && bb->out_v[x->u.variable->cnt] != UNDEF &&
        bb->out_v[x->u.variable->cnt] != NAC) {
        x->kind = OP_CONSTANT;
        x->u.value = bb->out_v[x->u.variable->cnt];
        return 1;
    }
    return 0;
}

void reduce_constant() {
    for (int i = 0; i < num_funcs; ++i) {
        int safe_exit = 0;
        const_change = 1;
        while (const_change && safe_exit < 10) {
            const_change = 0;
            BasicBlock_ptr bb = funcs[i].enter;
            while (bb) {
                update_bb(bb);
                is_changed(bb);
                bb = bb->adj_to;
                if (bb->is_finish) break;
            }
            update_bb(bb);
            is_changed(bb);
            safe_exit++;
        }
        // log_cfg();
        // printf("safe_exit = %d\n", safe_exit);
        InterCodes p = funcs[i].enter->adj_to->start;
        while (p && p->code->kind != IR_FUNC) {
            if (!p->code->disabled) {
                if (p->code->kind == IR_ASSIGN || p->code->kind == IR_ADD ||
                    p->code->kind == IR_SUB || p->code->kind == IR_DIV ||
                    p->code->kind == IR_MUL) {
                    if (check_op_const(p->code->x, funcs[i].exit)) {
                        p->code->disabled = 1;
                    } else {
                        check_op_const(p->code->y, funcs[i].exit);
                        check_op_const(p->code->z, funcs[i].exit);
                    }
                } else {
                    check_op_const(p->code->x, funcs[i].exit);
                    check_op_const(p->code->y, funcs[i].exit);
                    check_op_const(p->code->z, funcs[i].exit);
                }
            }
            p = p->prev;
        }
    }
}

/*--------------------------------------------------------------------
 * cfg.c
 *------------------------------------------------------------------*/
