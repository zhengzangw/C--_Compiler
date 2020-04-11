/*
 * File: symbol.c
 * Project: C--_Compiler
 * File Created: 2020-04-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "symbol.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"

Symbol_ptr new_symbol(int region) {
    Symbol_ptr tmp = (Symbol_ptr)malloc(sizeof(Symbol));
    tmp->name = NULL;
    tmp->type = NULL;
    tmp->nxt = tmp->cross_nxt = tmp->compst_nxt = NULL;
    tmp->region = region;
    tmp->is_activate = 1;
    tmp->is_proto = 0;
	return tmp;
}

Symbol_ptr hash_table[SYMBOL_SIZE];
Symbol_ptr compst[COMPST_SIZE];

void compst_insert(Symbol_ptr cur) {
    if (!compst[cur->region]) {
        compst[cur->region] = cur;
    } else {
        cur->compst_nxt = compst[cur->region];
        compst[cur->region] = cur;
    }
}

void compst_destroy(int depth) {
    for (Symbol_ptr p = compst[depth]; p; p = p->compst_nxt) {
        p->is_activate = 0;
    }
    compst[depth] = NULL;
}

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        i = val & ~0x3fff;
        if (i) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

int hash_same(Symbol_ptr node_new, Symbol_ptr node_old) {
    if (!node_old->is_activate) {
        return 0;
    }
    if (strcmp(node_new->name, node_new->name) == 0 &&
        node_new->region <= node_old->region) {
        return 1;
    }
    return 0;
}

void hash_create() {
    for (int i = 0; i < SYMBOL_SIZE; ++i) {
        hash_table[i] = NULL;
    }
    for (int i = 0; i < COMPST_SIZE; ++i) {
        compst[i] = NULL;
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
            if (hash_same(node, cur)) {
                return 1;
            }
            cur = cur->nxt;
        }
        cur->nxt = node;
    }
    compst_insert(node);
    return 0;
}

Symbol* hash_search(char* name) {
    unsigned int index = hash_pjw(name);
    Symbol_ptr cur = hash_table[index];
    Symbol_ptr opt = NULL;
    while (cur) {
        if (strcmp(name, hash_table[index]->name) == 0) {
            if (!opt || opt->region < cur->region) {
                opt = cur;
            }
        }
        cur = cur->nxt;
    }
    return opt;
}

const char* kind_name[] = {"BASIC", "ARRAY", "STRUCT", "FUNC"};
const char* basic_name[] = {"UNKNOWN", "INT", "FLOAT"};
void log_symbol(Symbol_ptr node) {
    printf("%*s%s:", node->region * 2, "", node->name);
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
                if (i > 0) printf(", ");
                log_symbol(tmp);
                log_type(tmp->type);
                tmp = tmp->cross_nxt;
            }
            printf(")");
            break;
        case STRUCTURE:
            printf("{\n");
            tmp = type->u.structure;
            while (tmp) {
                log_symbol(tmp);
                log_type(tmp->type);
                tmp = tmp->cross_nxt;
                printf(";\n");
            }
            printf("}");
            break;
    }
}

void _hash_print_all_symbols() {
	printf("######## SYMBOLE TABLE ########\n");
    for (int i = 0; i < SYMBOL_SIZE; ++i) {
        Symbol_ptr cur = hash_table[i];
        while (cur) {
            log_symbol(cur);
            log_type(cur->type);
            printf("\n");
            cur = cur->nxt;
        }
    }
	printf("######## SYMBOLE TABLE ########\n");
}

void _compst_print_all_symbols(int region){
	for (Symbol_ptr p=compst[region]; p; p=p->nxt){
		log_symbol(p);
		log_type(p->type);
		printf("\n");
	}
}

/*--------------------------------------------------------------------
 * symbol.c
 *------------------------------------------------------------------*/
