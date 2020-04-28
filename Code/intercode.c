/*
 * File: intercode.c
 * Project: C--_Compiler
 * File Created: 2020-04-21
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "intercode.h"

#include "common.h"

/*** Insert Intercode ***/

InterCodes intercodes_s = NULL;
InterCodes intercodes_t = NULL;

void insert_intercode(InterCode ir) {
    InterCodes tmp_code = (InterCodes)malloc(sizeof(struct InterCodes_));
    // output_intercode(ir, stdout);
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

/*** Output Intercode ***/

void output_op(Operand op, FILE* fp) {
    switch (op->kind) {
        case OP_LABEL:
            fprintf(fp, "label%d", op->u.label_no);
            break;
        case OP_TEMP:
            fprintf(fp, "t%d", op->u.tmp_no);
            break;
        case OP_FUNCTION:
            fprintf(fp, "%s", op->u.val);
            break;
        case OP_CONSTANT:
            fprintf(fp, "#%s", op->u.val);
            break;
        case OP_INT:
            fprintf(fp, "#%d", op->u.value);
            break;
        case OP_VARIABLE:
            fprintf(fp, "v%d", op->u.variable->cnt);
            break;
        case OP_SIZE:
            fprintf(fp, "%d", op->u.size);
        default:
            break;
    }
}

#define output_arith(op)   \
    output_op(ir->x, fp);  \
    fputs(" := ", fp);     \
    output_op(ir->y, fp);  \
    fputs(" " op " ", fp); \
    output_op(ir->z, fp);  \
    break

#define output_assign(prefix) \
    output_op(ir->x, fp);     \
    fputs(" := " prefix, fp); \
    output_op(ir->y, fp);     \
    break

#define output_order(order) \
    fputs(order " ", fp);   \
    output_op(ir->x, fp);   \
    break

#define output_label(order) \
    fputs(order " ", fp);   \
    output_op(ir->x, fp);   \
    fputs(" :", fp);        \
    break

void output_intercode(InterCode ir, FILE* fp) {
    switch (ir->kind) {
        case IR_LABEL:
            output_label("LABEL");
        case IR_FUNC:
            output_label("FUNCTION");
        case IR_ASSIGN:
            output_assign("");
        case IR_ADD:
            output_arith("+");
        case IR_SUB:
            output_arith("-");
        case IR_MUL:
            output_arith("*");
        case IR_DIV:
            output_arith("/");
        case IR_GET_ADDR:
            output_assign("&");
        case IR_GET_VAL:
            output_assign("*");
        case IR_GOTO:
            output_order("GOTO");
        case IR_RET:
            output_order("RETURN");
        case IR_ARG:
            output_order("ARG");
        case IR_ARG_ADDR:
            fputs("ARG &", fp);
            output_op(ir->x, fp);
            break;
        case IR_CALL:
            output_assign("CALL ");
        case IR_PARAM:
            output_order("PARAM");
        case IR_READ:
            output_order("READ");
        case IR_WRITE:
            output_order("WRITE");
        case IR_DEC:
            fputs("DEC ", fp);
            output_op(ir->x, fp);
            fputs(" ", fp);
            output_op(ir->y, fp);
            break;
        case IR_ASSIGN_ADDR:
            fputs("*", fp);
            output_op(ir->x, fp);
            fputs(" := ", fp);
            output_op(ir->y, fp);
            break;
        case IR_RELOP:
            fputs("IF ", fp);
            output_op(ir->x, fp);
            fprintf(fp, " %s ", ir->relop);
            output_op(ir->y, fp);
            fputs(" GOTO ", fp);
            output_op(ir->z, fp);
            break;
        default:
            fputs("???", fp);
            break;
    }
    fputs("\n", fp);
}

void output_intercodes(FILE* fp) {
    InterCodes p = intercodes_t;
    while (p) {
        output_intercode(p->code, fp);
        p = p->prev;
    }
}

/*** Operand Creation ***/

int label_num = 1;
Operand new_label() {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_LABEL;
    tmp->u.label_no = label_num++;
    return tmp;
}

int temp_num = 1;
Operand new_temp() {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_TEMP;
    tmp->u.tmp_no = temp_num++;
    return tmp;
}

Operand new_const(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_CONSTANT;
    strcpy(tmp->u.val, val);
    return tmp;
}

Operand new_int(int val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_INT;
    tmp->u.tmp_no = val;
    return tmp;
}

Operand new_size(int val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_SIZE;
    tmp->u.size = val;
    return tmp;
}

Operand new_func(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_FUNCTION;
    strcpy(tmp->u.val, val);
    return tmp;
}

Operand new_var(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_VARIABLE;
    tmp->u.variable = hash_find(val, SEARCH_EASY);
    return tmp;
}

/*** Intercode Creation ***/

void new_ir_1(IR_TYPE type, Operand op1) {
    InterCode tmp = (InterCode)malloc(sizeof(struct InterCode_));
    tmp->kind = type;
    tmp->x = op1;
    insert_intercode(tmp);
}

void new_ir_2(IR_TYPE type, Operand op1, Operand op2) {
    if (type == IR_CALL && op1 == NULL) {
        op1 = new_temp();
    } else if (op1 == NULL)
        return;
    InterCode tmp = (InterCode)malloc(sizeof(struct InterCode_));
    tmp->kind = type;
    tmp->x = op1;
    tmp->y = op2;
    insert_intercode(tmp);
}

void new_ir_3(IR_TYPE type, Operand op1, Operand op2, Operand op3) {
    if (op1 == NULL) return;
    InterCode tmp = (InterCode)malloc(sizeof(struct InterCode_));
    tmp->kind = type;
    tmp->x = op1;
    tmp->y = op2;
    tmp->z = op3;
    insert_intercode(tmp);
}

void new_ir_if(char* relop, Operand op1, Operand op2, Operand op3) {
    InterCode tmp = (InterCode)malloc(sizeof(struct InterCode_));
    tmp->kind = IR_RELOP;
    tmp->x = op1;
    tmp->y = op2;
    tmp->z = op3;
    strcpy(tmp->relop, relop);
    insert_intercode(tmp);
}

/*--------------------------------------------------------------------
 * intercode.c
 *------------------------------------------------------------------*/
