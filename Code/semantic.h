/*
 * File: semantic.h
 * Project: C--_Compiler
 * File Created: 2020-04-04
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"

/*** High-Level Definitions ***/

void Program(AST_node* cur);
void ExtDefList(AST_node* cur);
void ExtDef(AST_node* cur);
void ExtDecList(AST_node* cur, Type_ptr specifier_type);

/*** Specifiers ***/

Type_ptr Specifier(AST_node* cur);
Type_ptr StructSpecifier(AST_node* cur);

/*** Declarators ***/

Symbol_ptr FunDec(AST_node* cur, Type_ptr specifier_type, int);
Symbol_ptr VarList(AST_node* cur, Symbol_ptr func);
Symbol_ptr ParamDec(AST_node* cur);
Symbol_ptr VarDec(AST_node* cur, Type_ptr specifier_type);

/*** Local Definitions ***/

Symbol_ptr DefList(AST_node* cur);
Symbol_ptr Def(AST_node* cur);
Symbol_ptr DecList(AST_node* cur, Type_ptr specifier_type);
Symbol_ptr Dec(AST_node* cur, Type_ptr specifier_type);

/*** Statments ***/

void CompSt(AST_node* cur);
void StmtList(AST_node* cur);
void Stmt(AST_node* cur);

/*** Expression ***/

Type_ptr Exp(AST_node* cur);

/*** System Function ***/

void initIO();

#endif

/*--------------------------------------------------------------------
 * semantic.h
 *------------------------------------------------------------------*/
