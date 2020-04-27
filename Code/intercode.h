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
#include "symbol.h"

/*** Operand ***/

typedef enum OP_TYPE_ {
    OP_LABEL,
    OP_TEMP,
    OP_VARIABLE,
    OP_CONSTANT,
    OP_INT,
    OP_FUNCTION,
    OP_SIZE
} OP_TYPE;
typedef struct Operand_* Operand;
struct Operand_ {
    OP_TYPE kind;
    union {
        int tmp_no;           // OP_TEMP
        Symbol_ptr variable;  // OP_VARIABLE
        int label_no;         // OP_LABEL
        char val[128];        // OP_FUNCTION, OP_CONSTANT
        int value;            // OP_CONSTANT
        int size;             // OP_SIZE
    } u;
};

Operand new_temp();            // OP_TEMP
Operand new_var(char* val);    // OP_VARIABLE
Operand new_label();           // OP_LABEL
Operand new_func(char* val);   // OP_FUNCTION
Operand new_int(int val);      // OP_CONSTANT
Operand new_const(char* val);  // OP_CONSTANT
Operand new_size(int val);     // OP_SIZE

/*** Intercode ***/

typedef enum IR_TYPE_ {
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
    IR_ARG_ADDR,
    IR_CALL,
    IR_PARAM,
    IR_READ,
    IR_WRITE
} IR_TYPE;

typedef struct InterCode_* InterCode;
struct InterCode_ {
    IR_TYPE kind;
    Operand x, y, z;
    char relop[64];
};

void new_ir_1(IR_TYPE type, Operand op1);
void new_ir_2(IR_TYPE type, Operand op1, Operand op2);
void new_ir_3(IR_TYPE type, Operand op1, Operand op2, Operand op3);
void new_ir_if(char* relop, Operand op1, Operand op2, Operand op3);

#define RETURN(i) new_ir_1(IR_RET, t##i)
#define LABEL(i) new_ir_1(IR_LABEL, label##i)
#define GOTO(i) new_ir_1(IR_GOTO, label##i)

/*** InterCodes ***/

typedef struct InterCodes_* InterCodes;
struct InterCodes_ {
    InterCode code;
    InterCodes prev, next;
};

void insert_intercode(InterCode ir);
void output_intercode(InterCode, FILE*);
void output_intercodes(FILE*);

#endif

/*--------------------------------------------------------------------
 * intercode.h
 *------------------------------------------------------------------*/
