/*
 * File: mips.c
 * Project: C--_Compiler
 * File Created: 2020-05-15
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "mips.h"

const int reg[] = {8, 9, 10, 11, 12, 13, 14, 15};

var_descriptor *addr_t, *addr_v;
int fp_offset = 0;
int cur_arg = 0;
int args_cnt = 0;
int param_count = 0;

/*** Data Segmentation ***/

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

/*** Register Selection ***/

#define li(reg_no, value) fprintf(fp, "  li $%d %d\n", reg_no, value)
#define lw(reg_no, value) fprintf(fp, "  lw $%d %d($fp)\n", reg_no, value)
#define sw(reg_no, value) fprintf(fp, "  sw $%d %d($fp)\n", reg_no, value)

#define reg_push(reg_no, op) reg_push_(reg_no, op, fp)
#define reg_push_x() reg_push_(reg[1], ir->x, fp)
void reg_push_(int reg_no, Operand op, FILE* fp) {
    var_descriptor* tmp;
    if (op->kind == OP_TEMP)
        tmp = &addr_t[op->u.tmp_no];
    else if (op->kind == OP_VARIABLE)
        tmp = &addr_v[op->u.variable->cnt];
    if (!tmp->offset) {
        fp_offset -= 4;
        tmp->offset = fp_offset;
    }
    sw(reg_no, tmp->offset);
}

#define reg_pop(reg_no, op) reg_pop_(reg_no, op, fp)
#define reg_pop_x() reg_pop_(reg[1], ir->x, fp)
#define reg_pop_y() reg_pop_(reg[2], ir->y, fp)
#define reg_pop_z() reg_pop_(reg[3], ir->z, fp)
void reg_pop_(int reg_no, Operand op, FILE* fp) {
    if (op->kind == OP_CONSTANT)
        li(reg_no, op->u.value);
    else if (op->kind == OP_TEMP)
        lw(reg_no, addr_t[op->u.tmp_no].offset);
    else if (op->kind == OP_VARIABLE)
        lw(reg_no, addr_v[op->u.variable->cnt].offset);
}

/*** Translation ***/

void arg_sp(FILE* fp, InterCodes p) {
    args_cnt = 0;
    while (p->code->kind == IR_ARG) {
        args_cnt++;
        p = p->prev;
    }
    cur_arg = args_cnt - 1;
    if (args_cnt >= 4) fprintf(fp, "  subu $sp, $sp, %d\n", 4 * (args_cnt - 4));
}

int frame_size;
int get_frame_size(InterCodes p) {
    frame_size = (temp_num + var_num + 1) * 4;
    return frame_size;
}

void output_mips_instruction(InterCodes irs, FILE* fp) {
    char* tmp;
    InterCode ir = irs->code;
    switch (ir->kind) {
        // Caller
        case IR_ARG:  // Caller: prepare args
            if (!args_cnt) arg_sp(fp, irs);
            if (ir->x->kind == OP_CONSTANT) {
                if (cur_arg < 4)
                    fprintf(fp, "  li $a%d, %d\n", cur_arg, ir->x->u.value);
                else {
                    reg_pop_x();
                    fprintf(fp, "  sw $t1 %d($sp)\n", 4 * (cur_arg - 4));
                }
            } else {
                reg_pop_x();
                if (cur_arg < 4)
                    fprintf(fp, "  move $a%d, $t1\n", cur_arg);
                else {
                    fprintf(fp, "  sw $t1 %d($sp)\n", 4 * (cur_arg - 4));
                }
            }
            cur_arg--;
            break;
        case IR_CALL:
            // reg_push(fp);  // Caller: store caller register
            fprintf(fp, "  jal %s\n", ir->y->u.val);  // jump to function
            fprintf(fp, "  move $t1 $v0\n");
            reg_push_x();
            if (args_cnt >= 4) {
                fprintf(fp, "  addi $sp $sp %d\n",
                        4 * (args_cnt - 4));  // restore $sp
            }
            args_cnt = 0;
            cur_arg = 0;
            // no need to restore caller register
            break;
        // Callee
        case IR_PARAM:
            if (param_count < 4) {
                fprintf(fp, "  move $t1 $%d\n", 4 + param_count);
                reg_push_x();
            } else {
                addr_v[ir->x->u.variable->cnt].offset =
                    4 + (param_count - 3) * 4;
            }
            param_count++;
            break;
        case IR_FUNC:
            param_count = 0;
            fprintf(fp, "\n%s:\n", ir->x->u.val);
            fprintf(fp, "  subu $sp $sp %d\n", 2 * 4);
            fprintf(fp, "  sw $ra 4($sp)\n");  // preserve return address
            fprintf(fp, "  sw $fp 0($sp)\n");  // preserve old fp
            fp_offset = 0;
            fprintf(fp, "  addi $fp $sp 0\n");  // load new fp
            fprintf(fp, "  subu $sp $sp %d\n",
                    get_frame_size(irs));  // frame size
            // no need to store callee register
            break;
        case IR_RET:
            if (ir->x->kind == OP_CONSTANT) {
                fprintf(fp, "  li $v0 %d\n", ir->x->u.value);
            } else {
                reg_pop_x();
                fprintf(fp, "  move $v0, $t1\n");
            }
            fprintf(fp, "  addi $sp $sp %d\n",
                    frame_size);               // frame size
            fprintf(fp, "  lw $fp 0($sp)\n");  // load old fp
            fprintf(fp, "  lw $ra 4($sp)\n");  // load return address
            fprintf(fp, "  addi $sp $sp %d\n", 2 * 4);
            fputs("  jr $ra\n", fp);
            break;
        // Array
        case IR_DEC:
            fp_offset -= ir->y->u.size;
            addr_v[ir->x->u.variable->cnt].offset = fp_offset;
            break;
        case IR_GET_ADDR:
            if (ir->y->kind == OP_TEMP)
                fprintf(fp, "  la $t1 %d($fp)\n",
                        addr_t[ir->y->u.tmp_no].offset);
            else
                fprintf(fp, "  la $t1 %d($fp)\n",
                        addr_v[ir->y->u.variable->cnt].offset);
            reg_push_x();
            break;
        // Special
        case IR_READ:
            fprintf(fp, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n");
            fprintf(fp, "  jal read\n  move $t1 $v0\n");
            reg_push_x();
            fprintf(fp, "  lw $ra 0($sp)\n  addi $sp $sp 4\n");
            break;
        case IR_WRITE:
            fprintf(fp, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n");
            reg_pop_x();
            fprintf(fp, "  move $a0, $t1\n");
            fprintf(fp, "  jal write\n");
            fprintf(fp, "  lw $ra 0($sp)\n  addi $sp $sp 4\n");
            break;
        // Assign & Arith
        case IR_ASSIGN:
            if (ir->y->kind == OP_CONSTANT) {
                li(reg[1], ir->y->u.value);
            } else {
                reg_pop_y();
                fprintf(fp, "  move $t1 $t2\n");
            }
            reg_push_x();
            break;
        case IR_ASSIGN_ADDR:
            reg_pop_y();
            reg_pop_x();
            fprintf(fp, "  sw $t2 0($t1)\n");
            break;
        case IR_GET_VAL:
            reg_pop_y();
            fprintf(fp, "  lw $t1 0($t2)\n");
            reg_push_x();
            break;
        case IR_ADD:
            if (ir->z->kind == OP_CONSTANT) {
                reg_pop_y();
                fprintf(fp, "  addi $t1 $t2 %d\n", ir->z->u.value);
            } else if (ir->y->kind == OP_CONSTANT) {
                reg_pop_z();
                fprintf(fp, "  addi $t1 $t3 %d\n", ir->y->u.value);
            } else {
                reg_pop_y();
                reg_pop_z();
                fprintf(fp, "  add $t1 $t2 $t3\n");
            }
            reg_push_x();
            break;
        case IR_SUB:
            reg_pop_y();
            if (ir->z->kind == OP_CONSTANT) {
                fprintf(fp, "  addi $t1 $t2 %d\n", -ir->z->u.value);
            } else {
                reg_pop_z();
                fprintf(fp, "  sub $t1 $t2 $t3\n");
            }
            reg_push_x();
            break;
        case IR_MUL:
            reg_pop_y();
            reg_pop_z();
            fprintf(fp, "  mul $t1 $t2 $t3\n");
            reg_push_x();
            break;
        case IR_DIV:
            reg_pop_y();
            reg_pop_z();
            fprintf(fp, "  div $t2 $t3\n  mflo $t1\n");
            reg_push_x();
            break;
        // Label & Goto
        case IR_LABEL:
            fprintf(fp, "label%d:\n", ir->x->u.label_no);
            break;
        case IR_GOTO:
            fprintf(fp, "  j label%d\n", ir->x->u.label_no);
            break;
        case IR_RELOP:
            if (strcmp(ir->relop, "==") == 0) {
                tmp = "beq";
            } else if (strcmp(ir->relop, "!=") == 0) {
                tmp = "bne";
            } else if (strcmp(ir->relop, ">") == 0) {
                tmp = "bgt";
            } else if (strcmp(ir->relop, "<") == 0) {
                tmp = "blt";
            } else if (strcmp(ir->relop, ">=") == 0) {
                tmp = "bge";
            } else if (strcmp(ir->relop, "<=") == 0) {
                tmp = "ble";
            }
            reg_pop_x();
            reg_pop_y();
            fprintf(fp, "  %s $t1 $t2 label%d\n", tmp, ir->z->u.label_no);
            break;
        default:
            // assert(0);
            break;
    }
}

void output_mips_instructions(InterCodes p, FILE* fp) {
    addr_t = (var_descriptor*)calloc(temp_num + 128, sizeof(var_descriptor));
    addr_v = (var_descriptor*)calloc(var_num + 128, sizeof(var_descriptor));
    output_data(fp);
    while (p) {
        if (!p->code->disabled) {
            output_mips_instruction(p, fp);
        }
        p = p->prev;
    }
}
