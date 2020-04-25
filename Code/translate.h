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

/*** High-Level Definitions ***/

void translate_Program(AST_node* cur);
void translate_ExtDefList(AST_node* cur);
void translate_ExtDef(AST_node* cur);
void translate_ExtDecList(AST_node* cur);

/*** Declarators ***/

void translate_FunDec(AST_node* cur);
void translate_VarList(AST_node* cur);
void translate_ParamDec(AST_node* cur);

/*** Statments ***/

void translate_CompSt(AST_node* cur);
void translate_StmtList(AST_node* cur);
void translate_Stmt(AST_node* cur);

/*** Expression ***/

void translate_Exp(AST_node* cur, Operand place);
void translate_Cond(AST_node* cur, Operand label_true, Operand label_false);
#define RETURN(i) new_ir_1(IR_RET, t##i)
#define LABEL(i) new_ir_1(IR_LABEL, label##i)
#define GOTO(i) new_ir_1(IR_GOTO, label##i)

#endif

/*--------------------------------------------------------------------
 * translate.h
 *------------------------------------------------------------------*/

