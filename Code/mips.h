/*
 * File: mips.c
 * Project: C--_Compiler
 * File Created: 2020-05-15
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "common.h"
#include "intercode.h"

typedef struct reg_descriptor_ {
    int is_temp;
} reg_descriptor;

typedef struct var_descriptor_ {
    int offset;
} var_descriptor;

void output_mips_instructions(InterCodes, FILE*);
