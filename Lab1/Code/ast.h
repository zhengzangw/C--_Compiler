/**
 *
 * C-- Compiler, Labs of NJU course Principles and Techiques of Compiler
 *
 * Author: Zangwei Zheng, zhengzangw@163.com
 *
 **/

#ifndef _AST_H
#define _AST_H

typedef struct abstract_syntax_tree{
    char name[64];
    char original_val[128];
    char val[128];
    int lineno;
    int child_num;
    struct abstract_syntax_tree* child[10];
} AST_node;

AST_node* ast_create(char* name, char* original_val, char *val, int lineno);
int ast_append(AST_node* cur, int child_num, ...);
void ast_print(AST_node* root, int depth);
extern AST_node* ast_root;

#endif

/*--------------------------------------------------------------------
 * ast.h
 *------------------------------------------------------------------*/