/*
 * File: translate.c
 * Project: C--_Compiler
 * File Created: 2020-04-21
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "translate.h"

#include <string.h>

#include "common.h"
#include "symbol.h"

/*** High-Level Definitions ***/

void translate_Program(AST_node* cur) {
    // Program -> ExtDefList
    if (astcmp(0, ExtDefList)) {
        translate_ExtDefList(cur->child[0]);
    }
}

void translate_ExtDefList(AST_node* cur) {
    // ExtDefList -> ExtDef ExtDefList
    translate_ExtDef(cur->child[0]);
    // ExtDefList -> \epsilon
    if (cur->child[1]) {
        translate_ExtDefList(cur->child[1]);
    }
}

void translate_ExtDef(AST_node* cur) {
    // ExtDef -> Specifier ExtDecList SEMI
    // ExtDef -> Specifier FunDec CompSt
    if (astcmp(1, FunDec) && astcmp(2, CompSt)) {
        translate_CompSt(cur->child[2]);
    }
    // ExtDef -> Specifier FunDec SEMI
    // ExtDef -> Specifier SEMI
}

/*** Declarators ***/

void translate_FunDec(AST_node* cur) {
    // FunDec -> ID LP RP
    // FunDec -> ID LP VarList RP
    if (cur->child_num == 4) {
        translate_VarList(cur->child[2]);
    }
}

void translate_VarList(AST_node* cur) {
    translate_ParamDec(cur->child[0]);
    // VarList -> ParamDec COMMA VarList
    if (cur->child_num == 3) {
        translate_VarList(cur->child[2]);
    }
    // VarList -> ParamDec
}

void translate_ParamDec(AST_node* cur) {}

/*** Statments ***/

void translate_CompSt(AST_node* cur) {
    // CompSt -> LC DefList StmtList RC
    if (cur->child[2]) {
        translate_StmtList(cur->child[2]);
    }
}

void translate_StmtList(AST_node* cur) {
    // StmtList -> Stmt StmtList
    translate_Stmt(cur->child[0]);
    if (cur->child[1]) {
        translate_StmtList(cur->child[1]);
    }
}

void translate_Stmt(AST_node* cur) {
    // Stmt -> Exp SEMI
    if (astcmp(0, Exp)) {
        translate_Exp(cur->child[0]);
    }
    // Stmt -> RETURN Exp SEMI
    else if (astcmp(0, RETURN)) {
        translate_Exp(cur->child[1]);
    }
    // Stmt -> WHILE LP Exp RP Stmt
    // Stmt -> IF LP Exp RP Stmt
    else if (cur->child_num == 5) {
        translate_Exp(cur->child[2]);
        translate_Stmt(cur->child[4]);
    }
    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    else if (cur->child_num == 7) {
        translate_Exp(cur->child[2]);
        translate_Stmt(cur->child[4]);
        translate_Stmt(cur->child[6]);
    }
    // Stmt -> CompSt
    else if (astcmp(0, CompSt)) {
        translate_CompSt(cur->child[0]);
    }
    return;
}

/*** Expression ***/

void translate_Exp(AST_node* cur) {
    // ID LP Args RP
    // ID LP RP
    if (astcmp(1, LP)) {
    }
    // LP Exp RP
    else if (astcmp(0, LP)) {
        translate_Exp(cur->child[1]);
    }
    // Exp LB Exp RB
    else if (astcmp(1, LB)) {
        translate_Exp(cur->child[0]);
    }
    // Exp DOT ID
    else if (astcmp(1, DOT)) {
        translate_Exp(cur->child[0]);
    }
    // Exp ASSIGNOP Exp
    else if (astcmp(1, ASSIGNOP)) {
        translate_Exp(cur->child[0]);
        translate_Exp(cur->child[2]);
    }
    // Exp AND Exp
    // Exp OR Exp
    // Exp RELOP Exp
    // Exp PLUS Exp
    // Exp MINUS Exp
    // Exp STAR Exp
    // Exp DIV Exp
    else if (cur->child_num == 3) {
        translate_Exp(cur->child[0]);
        translate_Exp(cur->child[2]);
    }
    // MINUS Exp
    // NOT Exp
    else if (cur->child_num == 2) {
        translate_Exp(cur->child[1]);
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
 * translate.c
 *------------------------------------------------------------------*/
