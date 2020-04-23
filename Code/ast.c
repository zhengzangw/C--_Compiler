/*
 * File: ast.c
 * Project: C--_Compiler
 * File Created: 2020-02-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "ast.h"
#include "common.h"

AST_node* ast_root = NULL;

int cnt = 0;

AST_node* ast_create(char* name, char* original_val, char* val, int lineno) {
    cnt++;
    AST_node* ret = (AST_node*)malloc(sizeof(AST_node));
    strcpy(ret->name, name);
    strcpy(ret->original_val, original_val);
    strcpy(ret->val, val);
    ret->lineno = lineno;
    ret->child_num = 0;
    return ret;
}

int ast_append(AST_node* cur, int child_num, ...) {
    va_list ap;
    va_start(ap, child_num);
    cur->child_num = child_num;
    for (int i = 0; i < child_num; ++i) {
        cur->child[i] = va_arg(ap, AST_node*);
    }
    va_end(ap);
    return 0;
}

#define blank(n) \
    for (int i = 0; i < 2 * n; ++i) printf(" ");
void ast_print(AST_node* root, int depth) {
    if (root == NULL) return;
    blank(depth);
    if (root->child_num) {
        printf("%s (%d)\n", root->name, root->lineno);
        for (int i = 0; i < root->child_num; ++i) {
            ast_print(root->child[i], depth + 1);
        }
    } else {
        if (strcmp(root->name, "ID") == 0 || strcmp(root->name, "INT") == 0 ||
            strcmp(root->name, "FLOAT") == 0 ||
            strcmp(root->name, "TYPE") == 0) {
            printf("%s: %s\n", root->name, root->val);
        } else {
            printf("%s\n", root->name);
        }
    }
}

/*--------------------------------------------------------------------
 * ast.c
 *------------------------------------------------------------------*/
