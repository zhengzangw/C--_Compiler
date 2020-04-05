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

void Program(AST_node* cur);
void ExtDefList(AST_node* cur);
void ExtDef(AST_node* cur);
void FunDec(AST_node* cur, Type_ptr specifier_type);
Symbol_ptr VarList(AST_node* cur, Symbol_ptr);
Symbol_ptr ParamDec(AST_node* cur);
void CompSt(AST_node* cur);
void ExtDecList(AST_node* cur, Type_ptr specifier_type);
Symbol_ptr VarDec(AST_node* cur, Type_ptr specifier_type);
Type_ptr Specifier(AST_node* cur);

#endif

/*--------------------------------------------------------------------
 * semantic.h
 *------------------------------------------------------------------*/
