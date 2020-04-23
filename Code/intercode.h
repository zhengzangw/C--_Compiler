/*
 * File: intercode.h
 * Project: C--_Compiler
 * File Created: 2020-04-21
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#ifndef _INTERCODE_H
#define _INTERCODE_H

#include "common.h"

typedef struct Operand_* Operand;
struct Operend_ {
    enum {
        OP_VARIABLE,
        OP_CONSTANT,
        OP_ADDRESS,
		OP_LABEL,
		OP_FUNCTION
    } kind;
    union {
        int var_no;
        int value;
    } u;
};

typedef struct InterCode_* InterCode;
struct InterCode_ {
    enum {
        IR_LABEL,
        IR_FUNC,
        IR_ASSIGN,
        IR_ADD,
        IR_SUB,
        IR_MUL,
        IR_DIV,
        IR_GET_ADDR,
        IR_GET_VAL,
        IR_ASSIGN_ADDR,
        IR_GOTO,
        IR_RELOP,
        IR_RET,
        IR_DEC,
        IR_ARG,
        IR_CALL,
        IR_PARAM,
        IR_READ,
        IR_WRITE
    } kind;
    union {
        // Own operands
        struct {
            Operand x;
        } op_1;
        // Two operands
        struct {
            Operand x, y;
        } op_2;
        // Three operands
        struct {
            Operand x, y, z;
        } op_3;
        // IF
        struct {
            Operand x, y, z;
            char relop[64];
        } op_if;
        struct {
            Operand x;
            int size;
        } op_dec;
    } u;
};

typedef struct InterCodes_* InterCodes;
struct InterCodes_ {
    InterCode code;
    InterCodes prev, next;
};

void insert_intercode(InterCode ir);
void output_intercode(FILE*);

#endif

/*--------------------------------------------------------------------
 * intercode.h
 *------------------------------------------------------------------*/
