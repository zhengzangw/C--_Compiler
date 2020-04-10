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

#define astcmp(i, str) \
    (cur->child_num > (i) && strcmp(cur->child[i]->name, #str) == 0)

/*** High-Level Definitions ***/

void Program(AST_node* cur) {
    // Program -> ExtDefList
    if (astcmp(0, ExtDefList)) {
        ExtDefList(cur->child[0]);
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
    if (astcmp(1, ExtDecList)) {
        Type_ptr type = Specifier(cur->child[0]);
        ExtDecList(cur->child[1], type);
    }
    // ExtDef -> Specifier FunDec CompSt
    else if (astcmp(1, FunDec)) {
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
    if (astcmp(0, TYPE)) {
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
    if (astcmp(0, VarDec)) {
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
	if (cur->child_num == 3){
		Exp(cur->child[2]);
	}
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

void Stmt(AST_node* cur) {
    // Stmt -> Exp SEMI
    if (astcmp(0, Exp)) {
        Exp(cur->child[0]);
    }
    // Stmt -> RETURN Exp SEMI
    else if (astcmp(0, RETURN)) {
        Exp(cur->child[1]);
    }
    // Stmt -> WHILE LP Exp RP Stmt
    // Stmt -> IF LP Exp RP Stmt
    else if (cur->child_num == 5) {
        Exp(cur->child[2]);
        Stmt(cur->child[4]);
    }
    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    else if (cur->child_num == 7) {
        Exp(cur->child[2]);
        Stmt(cur->child[4]);
        Stmt(cur->child[6]);
    }
    // Stmt -> CompSt
    else if (astcmp(0, CompSt)) {
        CompSt(cur->child[0]);
    }
    return;
}

/*** Expression ***/

void Exp(AST_node* cur) {
    // ID LP Args RP
    // ID LP RP
    if (astcmp(1, LP)) {
        return;
    }
    // LP Exp RP
    else if (astcmp(0, LP)) {
        Exp(cur->child[1]);
    }
    // Exp LB Exp RB
    else if (astcmp(1, LB)) {
        Exp(cur->child[0]);
        Exp(cur->child[2]);
    }
    // Exp DOT ID
    else if (astcmp(1, DOT)) {
        Exp(cur->child[0]);
    }
    // Exp ASSIGNOP Exp
    else if (astcmp(1, ASSIGNOP)) {
        Exp(cur->child[1]);
    }
    // Exp AND Exp
    // Exp OR Exp
    // Exp RELOP Exp
    // Exp PLUS Exp
    // Exp MINUS Exp
    // Exp STAR Exp
    // Exp DIV Exp
    else if (cur->child_num == 3) {
        Exp(cur->child[1]);
    }
    // MINUS Exp
    // NOT Exp
    else if (cur->child_num == 2) {
        Exp(cur->child[1]);
    }
    // ID
    else if (astcmp(0, ID)) {
    }
    // INT
    else if (astcmp(0, INT)) {
    }
    // FLOAT
    else if (astcmp(0, FLOAT)) {
    }
}

/*--------------------------------------------------------------------
 * semantic.c
 *------------------------------------------------------------------*/
