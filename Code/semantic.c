/*
 * File: semantic.c
 * Project: C--_Compiler
 * File Created: 2020-04-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "semantic.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"

void Program(AST_node* cur) {
    // Program -> ExtDefList
    for (int i = 0; i < cur->child_num; ++i) {
        if (strcmp(cur->child[i]->name, "ExtDefList") == 0) {
            ExtDefList(cur->child[i]);
        }
    }
}

void ExtDefList(AST_node* cur) {
    // ExtDefList -> ExtDef ExtDefList
    ExtDef(cur->child[0]);
    // ExtDefList -> \epsilon
    if (cur->child[1]) {
        ExtDefList(cur->child[1]);
    }
}

void ExtDef(AST_node* cur) {
    // ExtDef -> Specifier ExtDecList SEMI
    if (strcmp(cur->child[1]->name, "ExtDecList") == 0) {
        Type_ptr type = Specifier(cur->child[0]);
        ExtDecList(cur->child[1], type);
    }
    // ExtDef -> Specifier FunDec CompSt
    else if (strcmp(cur->child[1]->name, "FunDec") == 0) {
        Type_ptr type = Specifier(cur->child[0]);
        FunDec(cur->child[1], type);
    }
    // ExtDef -> Specifier SEMI
    else {
        return;
    }
};

void FunDec(AST_node* cur, Type_ptr specifier_type) {
    Symbol_ptr tmp = (Symbol_ptr)malloc(sizeof(Symbol));
    tmp->name = cur->child[0]->val;
    tmp->type = (Type_ptr)malloc(sizeof(Type));
    tmp->type->kind = FUNCTION;
    tmp->type->u.function.ret = specifier_type;
    hash_insert(tmp);
    // FunDec -> ID LP RP
    if (cur->child_num == 3) {
        tmp->type->u.function.params_num = 0;
        tmp->type->u.function.params = NULL;
    }
    // FunDec -> ID LP VarList RP
    else {
        tmp->type->u.function.params_num = 0;
        tmp->type->u.function.params = VarList(cur->child[2], tmp);
    }
}

Symbol_ptr VarList(AST_node* cur, Symbol_ptr func) {
    func->type->u.function.params_num++;
    Symbol_ptr tmp = ParamDec(cur->child[0]);
    hash_insert(tmp);
    // VarList -> ParamDec COMMA VarList
    if (cur->child_num == 3) {
        tmp->func_nxt = VarList(cur->child[2], func);
    }
    // VarList -> ParamDec
    else {
        tmp->func_nxt = NULL;
    }
    return tmp;
}

Symbol_ptr ParamDec(AST_node* cur) {
    Type_ptr type = Specifier(cur->child[0]);
    Symbol_ptr ret = VarDec(cur->child[1], type);
    return ret;
}

void CompSt(AST_node* cur) { return; }

void ExtDecList(AST_node* cur, Type_ptr specifier_type) {
    // ExtDecList -> VarDec
    Symbol_ptr tmp = VarDec(cur->child[0], specifier_type);
    hash_insert(tmp);
    // ExtDecList -> VarDec COMMA ExtDecList
    if (cur->child_num == 3) ExtDecList(cur->child[2], specifier_type);
}

Symbol_ptr VarDec(AST_node* cur, Type_ptr specifier_type) {
    // VarDec -> VarDec LB INT RB
    if (strcmp(cur->child[0]->name, "VarDec") == 0) {
        Symbol_ptr tmp = VarDec(cur->child[0], specifier_type);
        Type_ptr tmp_type = (Type_ptr)malloc(sizeof(Type));
        tmp_type->kind = ARRAY;
        tmp_type->u.array.size = atoi(cur->child[2]->val);
        tmp_type->u.array.elem = tmp->type;
        tmp->type = tmp_type;
        return tmp;
    }
    // VarDec -> ID
    else {
        Symbol_ptr tmp = (Symbol_ptr)malloc(sizeof(Symbol));
        tmp->name = cur->child[0]->val;
        tmp->type = specifier_type;
        return tmp;
    }
}

Type_ptr Specifier(AST_node* cur) {
    Type_ptr type = (Type_ptr)malloc(sizeof(Type));
    // Specifier -> TYPE
    if (strcmp(cur->child[0]->name, "TYPE") == 0) {
        type->kind = BASIC;
        if (strcmp(cur->child[0]->val, "int") == 0) {
            type->u.basic = INT;
        } else {
            type->u.basic = FLOAT;
        }
    }
    // Specifier -> StructSpecifier
    else {
    }
    return type;
}

/*--------------------------------------------------------------------
 * semantic.c
 *------------------------------------------------------------------*/
