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

/*** High-Level Definitions ***/

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
        CompSt(cur->child[2]);
    }
    // ExtDef -> Specifier SEMI
    else {
        return;
    }
}

void ExtDecList(AST_node* cur, Type_ptr specifier_type) {
    // ExtDecList -> VarDec
    Symbol_ptr tmp = VarDec(cur->child[0], specifier_type);
    hash_insert(tmp);
    // ExtDecList -> VarDec COMMA ExtDecList
    if (cur->child_num == 3) ExtDecList(cur->child[2], specifier_type);
}

/*** Specifiers ***/

Type_ptr Specifier(AST_node* cur) {
    // Specifier -> TYPE
    if (strcmp(cur->child[0]->name, "TYPE") == 0) {
        Type_ptr type = (Type_ptr)malloc(sizeof(Type));
        type->kind = BASIC;
        if (strcmp(cur->child[0]->val, "int") == 0) {
            type->u.basic = INT;
        } else {
            type->u.basic = FLOAT;
        }
        return type;
    }
    // Specifier -> StructSpecifier
    else {
        return StructSpecifier(cur->child[0]);
    }
}

Type_ptr StructSpecifier(AST_node* cur) {
    // StructSpecifier -> STRUCT Tag
    if (cur->child_num == 2) {
        Symbol_ptr struct_tmp = hash_search(cur->child[1]->val);
        return struct_tmp->type;
    }
    // StructSpecifier -> STRUCT OptTag LC DefList RC
    else {
        Type_ptr type = (Type_ptr)malloc(sizeof(Type));
        type->kind = STRUCTURE;
        type->u.structure = DefList(cur->child[3]);
        if (cur->child[1]) {
            Symbol_ptr prototype = (Symbol_ptr)malloc(sizeof(Symbol));
            prototype->name = cur->child[1]->child[0]->val;
            prototype->is_structrue = 1;
            prototype->type = type;
            hash_insert(prototype);
        }
        return type;
    }
}

/*** Declarators ***/

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
        tmp->cross_nxt = VarList(cur->child[2], func);
    }
    // VarList -> ParamDec
    else {
        tmp->cross_nxt = NULL;
    }
    return tmp;
}

Symbol_ptr ParamDec(AST_node* cur) {
    Type_ptr type = Specifier(cur->child[0]);
    Symbol_ptr ret = VarDec(cur->child[1], type);
    return ret;
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

/*** Local Definitions ***/
Symbol_ptr DecList_last_ptr = NULL;
Symbol_ptr DefList(AST_node* cur) {
    // DefList -> \epsilon
    Symbol_ptr tmp = Def(cur->child[0]);
    Symbol_ptr cur_last_ptr = DecList_last_ptr;
    DecList_last_ptr = NULL;
    // DefList -> Def DefList
    if (cur->child[1]) {
        cur_last_ptr->cross_nxt = DefList(cur->child[1]);
    }
    return tmp;
}

Symbol_ptr Def(AST_node* cur) {
    // Def -> Specifier DecList SEMI
    Type_ptr type = Specifier(cur->child[0]);
    return DecList(cur->child[1], type);
}

Symbol_ptr DecList(AST_node* cur, Type_ptr specifier_type) {
    // DecList -> Dec
    Symbol_ptr tmp = Dec(cur->child[0], specifier_type);
    hash_insert(tmp);
    // DecList -> Dec COMMA DecList
    if (cur->child_num == 3) {
        tmp->cross_nxt = DecList(cur->child[2], specifier_type);
    }
    if (DecList_last_ptr == NULL) {
        DecList_last_ptr = tmp;
    }
    return tmp;
}

Symbol_ptr Dec(AST_node* cur, Type_ptr specifier_type) {
    // Dec -> VarDec
    Symbol_ptr tmp = VarDec(cur->child[0], specifier_type);
    // Dec -> VarDec ASSIGNOP Exp
    Exp(cur->child[2]);
    return tmp;
}

/*** Statments ***/

void CompSt(AST_node* cur) {
    // CompSt -> LC DefList StmtList RC
    if (cur->child[1]) {
        DefList(cur->child[1]);
    }
    if (cur->child[2]) {
        StmtList(cur->child[2]);
    }
}
void StmtList(AST_node* cur) {
    // StmtList -> Stmt StmtList
    Stmt(cur->child[0]);
    if (cur->child[1]) {
        StmtList(cur->child[1]);
    }
}
void Stmt(AST_node* cur) { return; }

/*** Expression ***/

void Exp(AST_node* cur) { return; }

/*--------------------------------------------------------------------
 * semantic.c
 *------------------------------------------------------------------*/
