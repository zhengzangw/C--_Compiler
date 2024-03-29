/*
 * File: translate.c
 * Project: C--_Compiler
 * File Created: 2020-04-21
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * Copyright 2020 NJU, Zangwei Zheng
 */

#include "translate.h"

#include "common.h"
#include "intercode.h"
#include "symbol.h"

/*** Assitant Function ***/

Type_ptr exp_type = NULL;

int calculate_Array(Type_ptr p) {
    if (p->u.array.base_size > 0) return p->u.array.base_size;
    if (p->kind == ARRAY) {
        return p->u.array.base_size =
                   p->u.array.size * calculate_Array(p->u.array.elem);
    } else if (p->kind == STRUCTURE) {
        return calculate_Struct(p);
    } else {
        return 4;
    }
}

int calculate_Struct_Offset(Type_ptr s, char* id) {
    int size = 0;
    for (Symbol_ptr p = s->u.structure; p; p = p->cross_nxt) {
        if (strcmp(p->name, id) == 0) break;
        switch (p->type->kind) {
            case BASIC:
                size += 4;
                break;
            case ARRAY:
                size += calculate_Array(p->type);
                break;
            case STRUCTURE:
                size += calculate_Struct(p->type);
                break;
            default:
                break;
        }
    }
    return size;
}

int calculate_Struct(Type_ptr s) { return calculate_Struct_Offset(s, ""); }

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
            // INT
            if (varlist->child[0]->child[1]->child_num == 1) {
                new_ir_1(IR_PARAM,
                         new_var(varlist->child[0]->child[1]->child[0]->val));
            }
            // Array
            else {
                AST_node* vardec = varlist->child[0]->child[1];
                AST_node* name = vardec->child[0];
                while (strcmp(name->name, "ID") != 0) name = name->child[0];
                calculate_Array(hash_find(name->val, SEARCH_EASY)->type);
                new_ir_1(IR_PARAM, new_var(name->val));
            }

            if (varlist->child_num == 3) {
                varlist = varlist->child[2];
            } else {
                break;
            }
        }
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
        } else if (tmp->type->kind == STRUCTURE) {
            new_ir_2(IR_DEC, new_var(tmp->name),
                     new_size(calculate_Struct(tmp->type)));
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
        translate_DecList(cur->child[2]);
    }
}

void translate_Dec(AST_node* cur) {
    // Dec -> VarDec
    translate_VarDec(cur->child[0]);
    // Dec -> VarDec ASSIGNOP Exp
    if (cur->child_num == 3) {
        Operand t1 = new_temp();
        translate_Exp(cur->child[2], t1, false);
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
    exp_type = NULL;
    // Stmt -> Exp SEMI
    if (astcmp(0, Exp)) {
        translate_Exp(cur->child[0], NULL, false);
    }
    // Stmt -> RETURN Exp SEMI
    else if (astcmp(0, RETURN)) {
        Operand t1 = new_temp();
        translate_Exp(cur->child[1], t1, false);
        RETURN(1);
    }
    // Stmt -> WHILE LP Exp RP Stmt
    else if (astcmp(0, WHILE)) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        LABEL(1);
        translate_Cond(cur->child[2], label2, label3);
        LABEL(2);
        translate_Stmt(cur->child[4]);
        GOTO(1);
        LABEL(3);
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
        translate_Exp(cur->child[0], t1, false);
        translate_Exp(cur->child[2], t2, false);
        new_ir_if(cur->child[1]->val, t1, t2, label_true);
        GOTO(_false);
    } else
    // (other case)
    {
        Operand t1 = new_temp();
        translate_Exp(cur, t1, false);
        new_ir_if("!=", t1, new_const("0"), label_true);
        GOTO(_false);
    }
}

void translate_Args(AST_node* cur) {
    // Exp
    Operand t1 = new_temp();
    translate_Exp(cur->child[0], t1, false);
    // Exp COMMA Args
    if (cur->child_num > 1) translate_Args(cur->child[2]);
    new_ir_1(IR_ARG, t1);
}

#ifdef OP_ASSIGN_TO_VAR
int op_assign_to_var = 0;
#endif

void translate_Exp(AST_node* cur, Operand place, int is_left) {
    // ID LP Args RP
    // ID LP RP
    if (astcmp(1, LP)) {
        if (strcmp(cur->child[0]->val, "read") == 0) {
            new_ir_1(IR_READ, place);
        } else if (strcmp(cur->child[0]->val, "write") == 0) {
            Operand t1 = new_temp();
            translate_Exp(cur->child[2]->child[0], t1, false);
            new_ir_1(IR_WRITE, t1);
        } else {
            if (cur->child_num > 3) {
                translate_Args(cur->child[2]);
            }
            new_ir_2(IR_CALL, place, new_func(cur->child[0]->val));
        }
        exp_type = &INT_TYPE;  // Function will not return array & structure
    }
    // LP Exp RP
    else if (astcmp(0, LP)) {
        translate_Exp(cur->child[1], place, is_left);
    }
    // Exp LB Exp RB
    else if (astcmp(1, LB)) {
        // Get Address
        Operand t1 = new_temp();
        translate_Exp(cur->child[0], t1, is_left);
        Type_ptr tmp_exp_type = exp_type;
        // Get Index
        Operand t2 = new_temp();
        translate_Exp(cur->child[2], t2, false);
        // Get Offset
        Operand t3 = new_temp();
        exp_type = tmp_exp_type->u.array.elem;
        if (exp_type->kind == ARRAY) {
#ifdef OP_ARR_CONST
            if (t2->kind == OP_CONSTANT) {
                int c = t2->u.value * exp_type->u.array.base_size;
                if (!c)
                    *place = *t1;
                else
                    new_ir_3(IR_ADD, place, t1, new_int(c));
                return;
            }
#endif
            new_ir_3(IR_MUL, t3, t2, new_int(exp_type->u.array.base_size));
            new_ir_3(IR_ADD, place, t1, t3);
        } else {
#ifdef OP_ARR_CONST
            if (t2->kind == OP_CONSTANT) {
                int c;
                if (exp_type->kind == STRUCTURE)
                    c = t2->u.value * calculate_Struct(exp_type);
                else
                    c = t2->u.value * 4;
                if (is_left || exp_type->kind == ARRAY ||
                    exp_type->kind == STRUCTURE)
                    if (!c)
                        *place = *t1;
                    else
                        new_ir_3(IR_ADD, place, t1, new_int(c));
                else {
                    Operand t4 = new_temp();
                    if (!c)
                        *t4 = *t1;
                    else
                        new_ir_3(IR_ADD, t4, t1, new_int(c));
                    new_ir_2(IR_GET_VAL, place, t4);
                }
                return;
            }
#endif
            if (exp_type->kind == STRUCTURE)
                new_ir_3(IR_MUL, t3, t2, new_int(calculate_Struct(exp_type)));
            else
                new_ir_3(IR_MUL, t3, t2, new_int(4));
            if (is_left || exp_type->kind == ARRAY ||
                exp_type->kind == STRUCTURE)
                new_ir_3(IR_ADD, place, t1, t3);
            else {
                Operand t4 = new_temp();
                new_ir_3(IR_ADD, t4, t1, t3);
                new_ir_2(IR_GET_VAL, place, t4);
            }
        }
    }
    // Exp DOT ID
    else if (astcmp(1, DOT)) {
        // Get Address
        Operand t1 = new_temp();
        translate_Exp(cur->child[0], t1, is_left);
        // Get Offset
        char* id_name = cur->child[2]->val;
        int size = calculate_Struct_Offset(exp_type, id_name);
        exp_type = hash_find_struct(id_name, exp_type)->type;

        if (exp_type->kind == ARRAY) calculate_Array(exp_type);
        if (is_left || exp_type->kind == ARRAY || exp_type->kind == STRUCTURE)
            // Left Value
            new_ir_3(IR_ADD, place, t1, new_int(size));
        else {
            // Right Value
            Operand t3 = new_temp();
            new_ir_3(IR_ADD, t3, t1, new_int(size));
            new_ir_2(IR_GET_VAL, place, t3);
        }
    }
    // Exp ASSIGNOP Exp
    else if (astcmp(1, ASSIGNOP)) {
        Operand t1 = new_temp();
        Type_ptr tmp_exp_type;

        if (cur->child[0]->child_num == 1 &&
            strcmp(cur->child[0]->child[0]->name, "ID") == 0 &&
            hash_find(cur->child[0]->child[0]->val, SEARCH_EASY)->type->kind !=
                ARRAY) {
#ifdef OP_ASSIGN_TO_VAR
            if (strcmp(cur->child[2]->child[0]->name, "INT") != 0 &&
                strcmp(cur->child[2]->child[0]->name, "ID") != 0) {
                t1 = new_var(cur->child[0]->child[0]->val);
                op_assign_to_var = 1;
                translate_Exp(cur->child[2], t1, false);
                op_assign_to_var = 0;
                tmp_exp_type = exp_type;
            } else {
#endif
                translate_Exp(cur->child[2], t1, false);
                tmp_exp_type = exp_type;
                new_ir_2(IR_ASSIGN, new_var(cur->child[0]->child[0]->val), t1);
#ifdef OP_ASSIGN_TO_VAR
            }
#endif
        } else {
            translate_Exp(cur->child[2], t1, false);
            tmp_exp_type = exp_type;
            Operand t2 = new_temp();
            translate_Exp(cur->child[0], t2, true);
            if (exp_type->kind == ARRAY) {
                // SPECIAL CASE: array = array
                int size1 = calculate_Array(tmp_exp_type);
                int size2 = calculate_Array(exp_type);
                int size = size1 < size2 ? size1 : size2;
                Operand label1 = new_label();
                Operand label2 = new_label();
                Operand label3 = new_label();
                Operand t_max = new_temp();
                Operand t_temp = new_temp();
                new_ir_3(IR_ADD, t_max, t2, new_int(size));
                LABEL(1);
                new_ir_if("<", t2, t_max, label2);
                GOTO(3);
                LABEL(2);
                new_ir_2(IR_GET_VAL, t_temp, t1);
                new_ir_2(IR_ASSIGN_ADDR, t2, t_temp);
                new_ir_3(IR_ADD, t1, t1, new_int(4));
                new_ir_3(IR_ADD, t2, t2, new_int(4));
                GOTO(1);
                LABEL(3);
            } else {
                new_ir_2(IR_ASSIGN_ADDR, t2, t1);
            }
        }

        exp_type = tmp_exp_type;
#ifdef OP_TEMP_REPLACE
        if (place && place->kind == OP_TEMP && t1->kind == OP_TEMP) {
            place->u = t1->u;
            return;
        }
#endif
        new_ir_2(IR_ASSIGN, place, t1);
    }
    // Exp AND Exp
    // Exp OR Exp
    // Exp RELOP Exp
    // NOT Exp
    else if (astcmp(0, NOT) || astcmp(1, AND) || astcmp(1, OR) ||
             astcmp(1, RELOP)) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand now = place;
#ifdef OP_ASSIGN_TO_VAR
        if (op_assign_to_var) {
            now = new_temp();
        }
#endif
        new_ir_2(IR_ASSIGN, now, new_const("0"));
        translate_Cond(cur, label1, label2);
        LABEL(1);
        new_ir_2(IR_ASSIGN, now, new_const("1"));
        LABEL(2);
#ifdef OP_ASSIGN_TO_VAR
        if (op_assign_to_var) {
            new_ir_2(IR_ASSIGN, place, now);
        }
#endif
    }
    // Exp PLUS Exp
    // Exp MINUS Exp
    // Exp STAR Exp
    // Exp DIV Exp
    else if (cur->child_num == 3) {
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        translate_Exp(cur->child[0], t1, is_left);
        translate_Exp(cur->child[2], t2, is_left);
#ifdef OP_ARITH_CONST
        if (place && place->kind == OP_TEMP) {
            if (t1->kind == OP_CONSTANT && t2->kind == OP_CONSTANT) {
                int c;
                if (astcmp(1, PLUS))
                    c = t1->u.value + t2->u.value;
                else if (astcmp(1, MINUS))
                    c = t1->u.value - t2->u.value;
                else if (astcmp(1, STAR))
                    c = t1->u.value * t2->u.value;
                else if (astcmp(1, DIV)) {
                    // Special handle for a/0
                    if (!t2->u.value) c = 2147483647;
                    else c = t1->u.value / t2->u.value;
                }
                place->kind = OP_CONSTANT;
                place->u.value = c;
                return;
            }
        }
        if (t2->kind == OP_TEMP && t2->u.value == 0) {
            if (astcmp(1, PLUS) || astcmp(1, MINUS)) {
                place->kind = t1->kind;
                place->u = t1->u;
            } else if (astcmp(1, STAR)) {
                place->kind = OP_CONSTANT;
                place->u.value = 0;
            }
            return;
        }
        if (t1->kind == OP_TEMP && t1->u.value == 0) {
            if (astcmp(1, PLUS) || astcmp(1, MINUS)) {
                place->kind = t2->kind;
                place->u = t2->u;
            } else if (astcmp(1, STAR)) {
                place->kind = OP_CONSTANT;
                place->u.value = 0;
            }
            return;
        }
#endif
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
        translate_Exp(cur->child[1], t1, is_left);
        new_ir_3(IR_SUB, place, new_const("0"), t1);
    }
    // ID
    else if (astcmp(0, ID)) {
        Operand val = new_var(cur->child[0]->val);
        Symbol_ptr p = hash_find(cur->child[0]->val, SEARCH_EASY);
        exp_type = p->type;
        if ((exp_type->kind == ARRAY || exp_type->kind == STRUCTURE) &&
            !p->is_param) {
            new_ir_2(IR_GET_ADDR, place, val);
        } else {
#ifdef OP_ID
            if (place) {
                place->kind = val->kind;
                place->u = val->u;
            }
#else
            new_ir_2(IR_ASSIGN, place, val);
#endif
        }
    }
    // INT
    else if (astcmp(0, INT)) {
        exp_type = &INT_TYPE;
        Operand val = new_const(cur->child[0]->val);
#ifdef OP_INT
        if (place) {
            place->kind = val->kind;
            place->u = val->u;
        }
#else
        new_ir_2(IR_ASSIGN, place, val);
#endif
    }
    // FLOAT
    else if (astcmp(0, FLOAT)) {
        // Not considered
    }
}

/*--------------------------------------------------------------------
 * translate.c
 *------------------------------------------------------------------*/
