/*
 * File: symbol.h
 * Project: C--_Compiler
 * File Created: 2020-04-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#ifndef _SYMBOL_H
#define _SYMBOL_H

#define SYMBOL_SIZE 32767

typedef enum _BASIC_TYPE { UNKNOWN, INT, FLOAT } BASIC_TYPE;
typedef enum _KIND { BASIC, ARRAY, STRUCTURE, FUNCTION } KIND;
typedef struct _Type* Type_ptr;
typedef struct _Symbol* Symbol_ptr;
typedef struct _Type {
    KIND kind;
    union {
        // BASIC
        int basic;

        // ARRAY
        struct {
            Type_ptr elem;
            int size;
        } array;

        // STRUCTRURE
        Symbol_ptr structure;

        // FUNCTION
        struct {
            Type_ptr ret;
            int params_num;
            Symbol_ptr params;
        } function;
    } u;
} Type;

typedef struct _Symbol {
    char* name;
    Type_ptr type;
    Symbol_ptr nxt, cross_nxt;
    int is_structrue;
} Symbol;

extern Symbol* hash_table[SYMBOL_SIZE];
void hash_create();
int hash_insert();
Symbol* hash_search();
void _hash_print_all_symbols();

#endif

/*--------------------------------------------------------------------
 * symbol.h
 *------------------------------------------------------------------*/
