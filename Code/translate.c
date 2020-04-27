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

#include "common.h"
#include "intercode.h"
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
    /* Not Considered */
    // ExtDef -> Specifier FunDec CompSt
    if (astcmp(1, FunDec) && astcmp(2, CompSt)) {
        translate_FunDec(cur->child[1]);
        translate_CompSt(cur->child[2]);
    }
    // ExtDef -> Specifier FunDec SEMI
    // ExtDef -> Specifier SEMI
}

/*** Declarators ***/

void translate_FunDec(AST_node* cur) {
    // FunDec -> ID LP RP
    new_ir_1(IR_FUNC, new_func(cur->child[0]->val));
    // FunDec -> ID LP VarList RP
    if (cur->child_num == 4) {
        AST_node* varlist = cur->child[2];
        while (true) {
            if (varlist->child[0]->child[1]->child_num == 1) {
                new_ir_1(IR_PARAM,
                         new_var(varlist->child[0]->child[1]->child[0]->val));
            } else {
                new_ir_1(
                    IR_PARAM,
                    new_var(
                        varlist->child[0]->child[1]->child[0]->child[0]->val));
            }

            if (varlist->child_num == 3) {
                varlist = varlist->child[2];
            } else {
                break;
            }
        }
    }
}

int calculate_Array(Type_ptr p) {
    if (p->kind == ARRAY) {
        return p->u.array.base_size =
                   p->u.array.size * calculate_Array(p->u.array.elem);
    } else {
        return 4;
    }
}

void translate_VarDec(AST_node* cur) {
    // VarDec -> VarDec LB INT RB
    if (cur->child_num == 4) {
        translate_VarDec(cur->child[0]);
    }
    // VarDec -> ID
    else {
        Symbol_ptr tmp = hash_find(cur->child[0]->val, SEARCH_EASY);
        if (tmp->type->kind == ARRAY) {
            int size = calculate_Array(tmp->type);
            new_ir_2(IR_DEC, new_var(tmp->name), new_size(size));
        }
    }
}

/*** Local Definitions ***/

void translate_DefList(AST_node* cur) {
    // DefList -> \epsilon
    // DefList -> Def DefList
    translate_Def(cur->child[0]);
    if (cur->child[1]) {
        translate_DefList(cur->child[1]);
    }
}

void translate_Def(AST_node* cur) {
    // Def -> Specifier DecList SEMI
    translate_DecList(cur->child[1]);
}

void translate_DecList(AST_node* cur) {
    // DecList -> Dec
    translate_Dec(cur->child[0]);
    // DecList -> Dec COMMA DecList
    if (cur->child[2]) {
        translate_DefList(cur->child[2]);
    }
}

void translate_Dec(AST_node* cur) {
    // Dec -> VarDec
    translate_VarDec(cur->child[0]);
    // Dec -> VarDec ASSIGNOP Exp
    if (cur->child_num == 3) {
        Operand t1 = new_temp();
		translate_Exp(cur->child[2], t1);
        new_ir_2(IR_ASSIGN, new_var(cur->child[0]->child[0]->val), t1);
    }
}

/*** Statments ***/

void translate_CompSt(AST_node* cur) {
    // CompSt -> LC DefList StmtList RC
    if (cur->child[1]) {
        translate_DefList(cur->child[1]);
    }
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
        translate_Exp(cur->child[0], NULL);
    }
    // Stmt -> RETURN Exp SEMI
    else if (astcmp(0, RETURN)) {
        Operand t1 = new_temp();
        translate_Exp(cur->child[1], t1);
        RETURN(1);
    }
    // Stmt -> WHILE LP Exp RP Stmt
    else if (astcmp(0, WHILE)) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        LABEL(1);
        translate_Cond(cur->child[2], label1, label2);
        GOTO(3);
        LABEL(2);
        translate_Stmt(cur->child[4]);
        LABEL(2);
    }
    // Stmt -> IF LP Exp RP Stmt
    else if (cur->child_num == 5) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        translate_Cond(cur->child[2], label1, label2);
        LABEL(1);
        translate_Stmt(cur->child[4]);
        LABEL(2);
    }
    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    else if (cur->child_num == 7) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        translate_Cond(cur->child[2], label1, label2);
        LABEL(1);
        translate_Stmt(cur->child[4]);
        GOTO(3);
        LABEL(2);
        translate_Stmt(cur->child[6]);
        LABEL(3);
    }
    // Stmt -> CompSt
    else if (astcmp(0, CompSt)) {
        translate_CompSt(cur->child[0]);
    }
}

/*** Expression ***/

void translate_Cond(AST_node* cur, Operand label_true, Operand label_false) {
    // NOT Exp
    if (astcmp(0, NOT)) {
        translate_Cond(cur->child[1], label_false, label_true);
    }
    // Exp AND Exp
    else if (astcmp(1, AND)) {
        Operand label1 = new_label();
        translate_Cond(cur->child[0], label1, label_false);
        LABEL(1);
        translate_Cond(cur->child[2], label_true, label_false);
    }
    // Exp OR Exp
    else if (astcmp(1, OR)) {
        Operand label1 = new_label();
        translate_Cond(cur->child[0], label_true, label1);
        LABEL(1);
        translate_Cond(cur->child[2], label_true, label_false);
    }
    // Exp RELOP Exp
    else if (astcmp(1, RELOP)) {
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        translate_Exp(cur->child[0], t1);
        translate_Exp(cur->child[2], t2);
        new_ir_if(cur->child[1]->val, t1, t2, label_true);
        GOTO(_false);
    } else
    // (other case)
    {
        Operand t1 = new_temp();
        translate_Exp(cur, t1);
        new_ir_if("!=", t1, new_const("0"), label_true);
        GOTO(_false);
    }
}

// Assist Global Variable
Operand arg_lists[64];
int arg_list_num = 0;
Type_ptr array_assist_cur = NULL;
int is_left = false;

void translate_Args(AST_node* cur) {
    // Exp
    Operand t1 = new_temp();
    translate_Exp(cur->child[0], t1);
    arg_lists[arg_list_num++] = t1;
    // Exp COMMA Args
    if (cur->child_num > 1) translate_Args(cur->child[2]);
}

void translate_Exp(AST_node* cur, Operand place) {
    // ID LP Args RP
    // ID LP RP
    if (astcmp(1, LP)) {
        arg_list_num = 0;
        if (cur->child_num > 3) {
            translate_Args(cur->child[2]);
        }
        if (strcmp(cur->child[0]->val, "read") == 0) {
            new_ir_1(IR_READ, place);
        } else if (strcmp(cur->child[0]->val, "write") == 0) {
            new_ir_1(IR_WRITE, arg_lists[0]);
        } else {
            for (int i = 0; i < arg_list_num; ++i) {
                new_ir_1(IR_ARG, arg_lists[i]);
            }
            new_ir_2(IR_CALL, place, new_func(cur->child[0]->val));
        }
    }
    // LP Exp RP
    else if (astcmp(0, LP)) {
        translate_Exp(cur->child[1], place);
    }
    // Exp LB Exp RB
    else if (astcmp(1, LB)) {
        // Get Address
        Operand t1 = new_temp();
        if (cur->child[0]->child_num == 1 &&
            strcmp(cur->child[0]->child[0]->name, "ID") == 0) {
            Symbol_ptr p = hash_find(cur->child[0]->child[0]->val, SEARCH_EASY);
            array_assist_cur = p->type;
            if (p->is_param)
                new_ir_2(IR_ASSIGN, t1, new_var(cur->child[0]->child[0]->val));
            else
                new_ir_2(IR_GET_ADDR, t1,
                         new_var(cur->child[0]->child[0]->val));
        } else {
            translate_Exp(cur->child[0], t1);
        }
        // Get Index
        Operand t2 = new_temp();
        translate_Exp(cur->child[2], t2);
        // Get Offset
        Operand t3 = new_temp();
        if (array_assist_cur->u.array.elem->kind == ARRAY) {
            new_ir_3(
                IR_MUL, t3, t2,
                new_int(array_assist_cur->u.array.elem->u.array.base_size));
            new_ir_3(IR_ADD, place, t1, t3);
        } else {
            new_ir_3(IR_MUL, t3, t2, new_int(4));
            if (is_left)
                new_ir_3(IR_ADD, place, t1, t3);
            else {
                Operand t4 = new_temp();
                new_ir_3(IR_ADD, t4, t1, t3);
                new_ir_2(IR_GET_VAL, place, t4);
            }
        }
        array_assist_cur = array_assist_cur->u.array.elem;
    }
    // Exp DOT ID
    else if (astcmp(1, DOT)) {
        // TODO
    }
    // Exp ASSIGNOP Exp
    else if (astcmp(1, ASSIGNOP)) {
        Operand t1 = new_temp();
        translate_Exp(cur->child[2], t1);
        if (cur->child[0]->child_num == 1 &&
            strcmp(cur->child[0]->child[0]->name, "ID") == 0) {
            new_ir_2(IR_ASSIGN, new_var(cur->child[0]->child[0]->val), t1);
        } else if (cur->child[0]->child_num == 4 &&
                   strcmp(cur->child[0]->child[1]->name, "LB") == 0) {
            Operand t2 = new_temp();
            is_left = 1;
            translate_Exp(cur->child[0], t2);
            is_left = 0;
            new_ir_2(IR_ASSIGN_ADDR, t2, t1);
        }
        new_ir_2(IR_ASSIGN, place, t1);
        // TODO
    }
    // Exp AND Exp
    // Exp OR Exp
    // Exp RELOP Exp
    // NOT Exp
    else if (astcmp(0, NOT) || astcmp(1, AND) || astcmp(1, OR) ||
             astcmp(1, RELOP)) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        new_ir_2(IR_ASSIGN, place, new_const("0"));
        translate_Cond(cur, label1, label2);
        LABEL(1);
        new_ir_2(IR_ASSIGN, place, new_const("1"));
        LABEL(2);
    }
    // Exp PLUS Exp
    // Exp MINUS Exp
    // Exp STAR Exp
    // Exp DIV Exp
    else if (cur->child_num == 3) {
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        translate_Exp(cur->child[0], t1);
        translate_Exp(cur->child[2], t2);
        IR_TYPE arith_type;
        if (astcmp(1, PLUS))
            arith_type = IR_ADD;
        else if (astcmp(1, MINUS))
            arith_type = IR_SUB;
        else if (astcmp(1, STAR))
            arith_type = IR_MUL;
        else if (astcmp(1, DIV))
            arith_type = IR_DIV;
        new_ir_3(arith_type, place, t1, t2);
    }
    // MINUS Exp
    else if (cur->child_num == 2) {
        Operand t1 = new_temp();
        translate_Exp(cur->child[1], t1);
        new_ir_3(IR_SUB, place, new_const("0"), t1);
    }
    // ID
    else if (astcmp(0, ID)) {
        Operand val = new_var(cur->child[0]->val);
        new_ir_2(IR_ASSIGN, place, val);
    }
    // INT
    else if (astcmp(0, INT)) {
        Operand val = new_const(cur->child[0]->val);
        new_ir_2(IR_ASSIGN, place, val);
    }
    // FLOAT
    else if (astcmp(0, FLOAT)) {
        // Not considered
    }
}

/*--------------------------------------------------------------------
 * translate.c
 *------------------------------------------------------------------*/
