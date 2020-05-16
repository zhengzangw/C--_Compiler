/*
 * File: mips.c
 * Project: C--_Compiler
 * File Created: 2020-05-15
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "mips.h"

var_descriptor *addr_t, *addr_v;
reg_descriptor reg_des[36];
int reg_to_use = 8;
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

int reg_achieve(FILE* fp) {
    if (reg_des[reg_to_use].active && reg_des[reg_to_use].is_temp != -1) {
        var_descriptor* cur;
        if (reg_des[reg_to_use].is_temp) {
            cur = &addr_t[reg_des[reg_to_use].no];
        } else {
            cur = &addr_v[reg_des[reg_to_use].no];
        }
        if (!cur->offset) {
            fp_offset -= 4;
            cur->offset = fp_offset;
        }
        fprintf(fp, "  sw $%d %d($fp)\n", reg_to_use, cur->offset);
        // variable not in reg
        cur->reg = 0;
        // reg not contain variable
        reg_des[reg_to_use].active = 0;
    }
    int ret = reg_to_use;
    if (reg_to_use == 15)
        reg_to_use = 8;
    else
        reg_to_use++;
    return ret;
}

void reg_push(FILE* fp) {
    for (int i = 8; i <= 15; ++i) {
        reg_achieve(fp);
    }
}

void reg_deactivate() {
    for (int i = 8; i <= 15; ++i) {
        reg_des[i].active = false;
    }
}

#define reg(op) reg_(op, fp)
int reg_(Operand op, FILE* fp) {
    int ret = 0;
    if (op->kind == OP_CONSTANT) {
        ret = reg_achieve(fp);
        fprintf(fp, "  li $%d %d\n", ret, op->u.value);
        reg_des[ret].is_temp = -1;
    } else if (op->kind == OP_TEMP) {
        if (addr_t[op->u.tmp_no].reg)
            ret = addr_t[op->u.tmp_no].reg;
        else {
            ret = reg_achieve(fp);
            if (addr_t[op->u.tmp_no].offset)
                fprintf(fp, "  lw $%d %d($fp)\n", ret,
                        addr_t[op->u.tmp_no].offset);
            reg_des[ret].is_temp = 1;
            reg_des[ret].no = op->u.tmp_no;
            addr_t[op->u.tmp_no].reg = ret;
        }
    } else if (op->kind == OP_VARIABLE) {
        if (addr_v[op->u.variable->cnt].reg)
            ret = addr_v[op->u.variable->cnt].reg;
        else {
            ret = reg_achieve(fp);
            if (addr_v[op->u.variable->cnt].offset)
                fprintf(fp, "  lw $%d %d($fp)\n", ret,
                        addr_v[op->u.variable->cnt].offset);
            reg_des[ret].is_temp = 0;
            reg_des[ret].no = op->u.variable->cnt;
            addr_v[op->u.variable->cnt].reg = ret;
        }
    }
    reg_des[ret].active = 1;
    return ret;
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
    frame_size = (temp_num + var_num) * 4;
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
                    fprintf(fp, "  sw $%d %d($sp)\n", reg(ir->x),
                            4 * (cur_arg - 4));
                }
            } else {
                if (cur_arg < 4)
                    fprintf(fp, "  move $a%d, $%d\n", cur_arg, reg(ir->x));
                else {
                    fprintf(fp, "  sw $%d %d($sp)\n", reg(ir->x),
                            4 * (cur_arg - 4));
                }
            }
            cur_arg--;
            break;
        case IR_CALL:
            reg_push(fp);  // Caller: store caller register
            fprintf(fp, "  jal %s\n", ir->y->u.val);  // jump to function
            fprintf(fp, "  move $%d $v0\n", reg(ir->x));
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
                fprintf(fp, "  move $%d $%d\n", reg(ir->x), 4 + param_count);
            } else {
                addr_v[ir->x->u.variable->cnt].offset =
                    4 + (param_count - 3) * 4;
            }
            param_count++;
            break;
        case IR_FUNC:
            reg_deactivate();
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
                fprintf(fp, "  move $v0, $%d\n", reg(ir->x));
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
                fprintf(fp, "  la $%d %d($fp)\n", reg(ir->x),
                        addr_t[ir->y->u.tmp_no].offset);
            else
                fprintf(fp, "  la $%d %d($fp)\n", reg(ir->x),
                        addr_v[ir->y->u.variable->cnt].offset);
            break;
        // Special
        case IR_READ:
            fprintf(fp, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n");
            fprintf(fp, "  jal read\n  move $%d $v0\n", reg(ir->x));
            fprintf(fp, "  lw $ra 0($sp)\n  addi $sp $sp 4\n");
            break;
        case IR_WRITE:
            fprintf(fp, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n");
            fprintf(fp, "  move $a0, $%d\n", reg(ir->x));
            fprintf(fp, "  jal write\n");
            fprintf(fp, "  lw $ra 0($sp)\n  addi $sp $sp 4\n");
            break;
        // Assign & Arith
        case IR_ASSIGN:
            if (ir->y->kind == OP_CONSTANT) {
                fprintf(fp, "  li $%d %d\n", reg(ir->x), ir->y->u.value);
            } else {
                fprintf(fp, "  move $%d $%d\n", reg(ir->x), reg(ir->y));
            }

            break;
        case IR_ASSIGN_ADDR:
            fprintf(fp, "  sw $%d 0($%d)\n", reg(ir->y), reg(ir->x));
            break;
        case IR_GET_VAL:
            fprintf(fp, "  lw $%d 0($%d)\n", reg(ir->x), reg(ir->y));
            break;
        case IR_ADD:
            if (ir->y->kind == OP_CONSTANT) {
                fprintf(fp, "  addi $%d $%d %d\n", reg(ir->x), reg(ir->y),
                        ir->z->u.value);
            } else {
                fprintf(fp, "  add $%d $%d $%d\n", reg(ir->x), reg(ir->y),
                        reg(ir->z));
            }
            break;
        case IR_SUB:
            if (ir->y->kind == OP_CONSTANT) {
                fprintf(fp, "  addi $%d $%d %d\n", reg(ir->x), reg(ir->y),
                        ir->z->u.value);
            } else {
                fprintf(fp, "  sub $%d $%d $%d\n", reg(ir->x), reg(ir->y),
                        reg(ir->z));
            }
            break;
        case IR_MUL:
            fprintf(fp, "  mul $%d $%d $%d\n", reg(ir->x), reg(ir->y),
                    reg(ir->z));
            break;
        case IR_DIV:
            fprintf(fp, "  div $%d $%d\n", reg(ir->y), reg(ir->z));
            fprintf(fp, "  mflo $%d\n", reg(ir->x));
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
                tmp = "bgt";
            } else if (strcmp(ir->relop, ">=") == 0) {
                tmp = "bge";
            } else if (strcmp(ir->relop, "<=") == 0) {
                tmp = "ble";
            }
            fprintf(fp, "  %s $%d $%d label%d\n", tmp, reg(ir->x), reg(ir->y),
                    ir->z->u.label_no);
            break;
        default:
            assert(0);
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
