/*
 * File: translate.h
 * Project: C--_Compiler
 * File Created: 2020-04-21
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "ast.h"
#include "intercode.h"

int calculate_Struct(Type_ptr s);
int calculate_Struct_Offset(Type_ptr s, char* id);
int calculate_Array(Type_ptr p);

/*** High-Level Definitions ***/

void translate_Program(AST_node* cur);
void translate_ExtDefList(AST_node* cur);
void translate_ExtDef(AST_node* cur);
void translate_ExtDecList(AST_node* cur);

/*** Declarators ***/

void translate_FunDec(AST_node* cur);
void translate_VarDec(AST_node* cur);

/*** Local Definitions ***/

void translate_DefList(AST_node* cur);
void translate_Def(AST_node* cur);
void translate_DecList(AST_node* cur);
void translate_Dec(AST_node* cur);

/*** Statments ***/

void translate_CompSt(AST_node* cur);
void translate_StmtList(AST_node* cur);
void translate_Stmt(AST_node* cur);

/*** Expression ***/

void translate_Exp(AST_node* cur, Operand place, int is_left);
void translate_Cond(AST_node* cur, Operand label_true, Operand label_false);

#endif

/*--------------------------------------------------------------------
 * translate.h
 *------------------------------------------------------------------*/

