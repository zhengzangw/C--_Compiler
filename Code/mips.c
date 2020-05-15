/*
 * File: mips.c
 * Project: C--_Compiler
 * File Created: 2020-05-15
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "mips.h"

int *addr_t, *addr_v;
int fp_offset;

void output_data(FILE* fp) {
    fputs(
        ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz "
        "\"\\n\"\n",
        fp);
    fputs(".globl main\n.text\n", fp);
    fputs(
        "read:\n  li $v0, 4\n  la $a0, _prompt\n  syscall\n  li $v0, 5\n  "
        "syscall\n  jr $ra\n\nwrite:\n  li $v0, 1\n  syscall\n  li $v0, 4\n  "
        "la $a0, _ret\n  syscall\n  move $v0, $0\n  jr $ra\n",
        fp);
}

#define reg(op) reg_(op, fp)
int cur_reg = 0;
int reg_(Operand op, FILE* fp) {
    if (op->kind == OP_CONSTANT) {
        fprintf(fp, "  li $%dt, %d\n", cur_reg, op->u.value);
    } else if (op->kind == OP_TEMP) {
        fprintf(fp, "  lw $%dt, %d($fp)\n", cur_reg, addr_t[op->u.tmp_no]);
    } else if (op->kind == OP_VARIABLE) {
        fprintf(fp, "  lw $%dt, %d($fp)\n", cur_reg,
                addr_v[op->u.variable->cnt]);
    }
    int ret = cur_reg;
    cur_reg = (cur_reg + 1) % 8;
    return cur_reg;
}

void reg_sw(int no, Operand op, FILE* fp) {
    if (op->kind == OP_TEMP) {
        if (!addr_t[op->u.tmp_no]) {
            addr_t[op->u.tmp_no] = ++fp_offset;
            fprintf(fp, "\n  addi $sp, $sp, -4");
        }
        fprintf(fp, "\n  sw $%dt %d($fp)", no, addr_t[op->u.tmp_no]);
    } else {
        if (!addr_v[op->u.variable->cnt]) {
            addr_v[op->u.variable->cnt] = ++fp_offset;
            fprintf(fp, "\n  addi $sp, $sp, -4");
        }
        fprintf(fp, "\n  sw $%dt %d($fp)", no, addr_v[op->u.variable->cnt]);
    }
}

void output_mips_instruction(InterCode ir, FILE* fp) {
    char* tmp;
    int regx, regy;
    if (ir->x) regx = reg(ir->x);
    switch (ir->kind) {
        // Label & Goto
        case IR_LABEL:
            fprintf(fp, "label%d:", ir->x->u.label_no);
            break;
        case IR_GOTO:
            fprintf(fp, "  j label%d", ir->x->u.label_no);
            break;
        case IR_RELOP:
            if (strcmp(ir->relop, "==") == 0) {
                tmp = "beq";
            } else if (strcmp(ir->relop, "!=") == 0) {
                tmp = "bne";
            } else if (strcmp(ir->relop, ">") == 0) {
                tmp = "bgt";
            } else if (strcmp(ir->relop, "<") == 0) {
                tmp = "bgt";
            } else if (strcmp(ir->relop, ">=") == 0) {
                tmp = "bge";
            } else if (strcmp(ir->relop, "<=") == 0) {
                tmp = "ble";
            }
            fprintf(fp, "  %s $%dt, $%dt, label%d", tmp, regx, reg(ir->y),
                    ir->z->u.label_no);
            break;
        // Func & Call
        case IR_FUNC:
            // TODO: add frame process
            fprintf(fp, "\n%s:", ir->x->u.val);
            break;
        case IR_ARG:
            break;
        case IR_ARG_ADDR:
            break;
        case IR_CALL:
            fprintf(fp, "  jal %s\n  move $%dt, $v0", ir->y->u.variable->name,
                    regx);
            break;
        case IR_PARAM:
            break;
        case IR_RET:
            if (ir->x->kind == OP_CONSTANT) {
                fprintf(fp, "  li $v0, %d\n", ir->x->u.value);
            } else {
                fprintf(fp, "  move $v0, $%dt\n", regx);
            }
            fputs("  jr $ra", fp);
            break;
        case IR_READ:
            fprintf(fp, "  jal read\n  move $%dt, $v0", regx);
            break;
        case IR_WRITE:
            fprintf(fp, "  jal write\n");
            break;
        // Assign & Arith
        case IR_ASSIGN:
            if (ir->y->kind == OP_CONSTANT) {
                fprintf(fp, "  li $%dt, %d", regx, ir->y->u.value);
            } else {
                fprintf(fp, "  li $%dt, $%dt", regx, reg(ir->y));
            }
            reg_sw(regx, ir->x, fp);
            break;
        case IR_ASSIGN_ADDR:
            fprintf(fp, "  sw $%dt, 0($%dt)", reg(ir->y), regx);
            reg_sw(regx, ir->x, fp);
            break;
        case IR_GET_VAL:
            fprintf(fp, "  lw $%dt, 0($%dt)", regx, reg(ir->y));
            reg_sw(regx, ir->x, fp);
            break;
        case IR_ADD:
            if (ir->y->kind == OP_CONSTANT) {
                fprintf(fp, "  addi $%dt, $%dt, %d", regx, reg(ir->y),
                        ir->y->u.value);
            } else {
                fprintf(fp, "  add $%dt, $%dt", regx, reg(ir->y));
            }
            reg_sw(regx, ir->x, fp);
            break;
        case IR_SUB:
            if (ir->y->kind == OP_CONSTANT) {
                fprintf(fp, "  addi $%dt, $%dt, %d", regx, reg(ir->y),
                        ir->y->u.value);
            } else {
                fprintf(fp, "  sub $%dt, $%dt", regx, reg(ir->y));
            }
            reg_sw(regx, ir->x, fp);
            break;
        case IR_MUL:
            fprintf(fp, "  mul $%dt, $%dt, %d", regx, reg(ir->y), reg(ir->z));
            reg_sw(regx, ir->x, fp);
            break;
        case IR_DIV:
            fprintf(fp, "  div $%dt, $%dt\n  mflo $%dt", reg(ir->y), reg(ir->z),
                    regx);
            reg_sw(regx, ir->x, fp);
            break;
        // Array
        case IR_DEC:
            break;
        case IR_GET_ADDR:
            if (ir->y->kind == OP_TEMP)
                fprintf(fp, "  la $%dt, %d($fp)", regx,
                        addr_t[ir->y->u.tmp_no]);
            else
                fprintf(fp, "  la $%dt, %d($fp)", regx,
                        addr_v[ir->y->u.variable->cnt]);
            reg_sw(regx, ir->x, fp);
            break;
        default:
            break;
    }
    fputs("\n", fp);
}

void output_mips_instructions(InterCodes p, FILE* fp) {
    addr_t = (int*)calloc(temp_num + 128, 0);
    addr_v = (int*)calloc(var_num + 128, 0);
    output_data(fp);
    while (p) {
        if (!p->code->disabled) {
            output_mips_instruction(p->code, fp);
        }
        p = p->prev;
    }
}
