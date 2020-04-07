/*
 * File: symbol.c
 * Project: C--_Compiler
 * File Created: 2020-04-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "symbol.h"

#include <string.h>

#include "common.h"

Symbol* hash_table[SYMBOL_SIZE];

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        i = val & ~0x3fff;
        if (i) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

void hash_create() {
    for (int i = 0; i < SYMBOL_SIZE; ++i) {
        hash_table[i] = NULL;
    }
}

int hash_insert(Symbol_ptr node) {
    node->nxt = NULL;
    unsigned int index = hash_pjw(node->name);
    if (hash_table[index] == NULL) {
        hash_table[index] = node;
    } else {
        Symbol_ptr cur = hash_table[index];
        while (cur->nxt) {
            cur = cur->nxt;
        }
        cur->nxt = node;
    }
    return 0;
}

Symbol* hash_search(char* name) {
    unsigned int index = hash_pjw(name);
    Symbol_ptr cur = hash_table[index];
    while (cur) {
        if (strcmp(name, hash_table[index]->name) == 0) {
            return hash_table[index];
        } else {
            cur = cur->nxt;
        }
    }
    return NULL;
}

const char* kind_name[] = {"BASIC", "ARRAY", "STRUCT", "FUNC"};
const char* basic_name[] = {"UNKNOWN", "INT", "FLOAT"};
void log_symbol(Symbol_ptr node) {
    if (node->is_structrue) {
        printf("%10s:%-6s:", node->name, "PROTO");
    } else {
        printf("%10s:%-6s:", node->name, kind_name[node->type->kind]);
    }
}
void log_type(Type_ptr type) {
    Symbol_ptr tmp;
    switch (type->kind) {
        case BASIC:
            printf("%s", basic_name[type->u.basic]);
            break;
        case ARRAY:
            printf("[%d]", type->u.array.size);
            log_type(type->u.array.elem);
            break;
        case FUNCTION:
            log_type(type->u.function.ret);
            printf("(");
            tmp = type->u.function.params;
            for (int i = 0; i < type->u.function.params_num; ++i) {
                if (i > 0) printf(",");
                log_symbol(tmp);
                log_type(tmp->type);
                tmp = tmp->cross_nxt;
            }
            printf(")");
            break;
        case STRUCTURE:
            printf("{");
            tmp = type->u.structure;
            while (tmp) {
                log_symbol(tmp);
                log_type(tmp->type);
                tmp = tmp->cross_nxt;
                printf(";");
            }
            printf("}");
            break;
    }
}
void _hash_print_all_symbols() {
    for (int i = 0; i < SYMBOL_SIZE; ++i) {
        Symbol_ptr cur = hash_table[i];
        while (cur) {
            log_symbol(cur);
            log_type(cur->type);
            printf("\n");
            cur = cur->nxt;
        }
    }
}

/*--------------------------------------------------------------------
 * symbol.c
 *------------------------------------------------------------------*/
