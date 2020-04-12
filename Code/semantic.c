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
#define semantic_error(type, lineno, desc) \
    printf("Error type %d at Line %d: %s.\n", type, lineno, desc)
#define semantic_error_option(type, lineno, desc, letter) \
    printf("Error type %d at Line %d: %s \"%s\".\n", type, lineno, desc, letter)

int region_depth = 0;
Symbol_ptr region_func = NULL;
int region_in_structure = 0;
Symbol_ptr claim_list[CLAIM_SIZE];
int total_claim = 0;

int claim_insert(Symbol_ptr func_node) {
    claim_list[total_claim++] = func_node;
    return 0;
}
void claim_check() {
    for (int i = 0; i < total_claim; ++i) {
        if (claim_list[i]->type->u.function.is_claim) {
            semantic_error(18, claim_list[i]->type->u.function.claim_lineno,
                           "Function claimed but not defined");
        }
    }
}

int equal_type(Type_ptr type_1, Type_ptr type_2) {
    if (type_1->kind == BASIC && type_1->u.basic == UNKNOWN) return 1;
    if (type_2->kind == BASIC && type_2->u.basic == UNKNOWN) return 1;
    Type_ptr type_nofunc_1 =
        type_1->kind == FUNCTION ? type_1->u.function.ret : type_1;
    Type_ptr type_nofunc_2 =
        type_2->kind == FUNCTION ? type_2->u.function.ret : type_2;
    if (type_nofunc_1->kind == type_nofunc_2->kind) {
        switch (type_nofunc_1->kind) {
            case BASIC:
                return type_nofunc_1->u.basic == type_nofunc_2->u.basic;
            case ARRAY: {
                Type_ptr t1, t2;
                for (t1 = type_nofunc_1->u.array.elem,
                    t2 = type_nofunc_2->u.array.elem;
                     t1->kind == ARRAY && t2->kind == ARRAY;
                     t1 = t1->u.array.elem, t2 = t2->u.array.elem)
                    ;
                return equal_type(t1, t2);
            }
            case STRUCTURE: {
                int ret = 1;
                for (Symbol_ptr p1 = type_nofunc_1->u.structure,
                                p2 = type_nofunc_2->u.structure;
                     p1 && p2; p1 = p1->cross_nxt, p2 = p2->cross_nxt) {
                    if (!equal_type(p1->type, p2->type)) {
                        ret = 0;
                        break;
                    }
                }
                return ret;
            }
            default:
                return -1;
        }
    }

    return 0;
}
int equal_func(Type_ptr func_1, Type_ptr func_2) {
    Symbol_ptr arg_1 = func_1->u.function.params;
    Symbol_ptr arg_2 = func_2->u.function.params;
    while (arg_1 && arg_2) {
        if (!equal_type(arg_1->type, arg_2->type)) return 0;
		arg_1 = arg_1->cross_nxt;
		arg_2 = arg_2->cross_nxt;
    }
	if (arg_1 || arg_2) return 0;
    return 1;
}

/*** High-Level Definitions ***/

void Program(AST_node* cur) {
    // Program -> ExtDefList
    if (astcmp(0, ExtDefList)) {
        ExtDefList(cur->child[0]);
    }
    // Error[18]
    claim_check();
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
    else if (astcmp(1, FunDec) && astcmp(2, CompSt)) {
        Type_ptr type = Specifier(cur->child[0]);
        region_func = FunDec(cur->child[1], type, false);
        CompSt(cur->child[2]);
        region_func = NULL;
    }
    // ExtDef -> Specifier FunDec SEMI
    else if (astcmp(1, FunDec) && astcmp(2, SEMI)) {
        Type_ptr type = Specifier(cur->child[0]);
        FunDec(cur->child[1], type, true);
    }
    // ExtDef -> Specifier SEMI
    else {
        Specifier(cur->child[0]);
    }
}

void ExtDecList(AST_node* cur, Type_ptr specifier_type) {
    // ExtDecList -> VarDec
    Symbol_ptr tmp = VarDec(cur->child[0], specifier_type);
    // Error[3]
    if (hash_insert(tmp)) {
        semantic_error_option(3, cur->child[0]->lineno, "Redefined variable",
                              tmp->name);
    }
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
        // Error[17]
        Symbol_ptr struct_tmp =
            hash_find(cur->child[1]->child[0]->val, SEARCH_PROTO);
        if (!struct_tmp) {
            semantic_error_option(17, cur->child[1]->lineno,
                                  "Undefined structure",
                                  cur->child[1]->child[0]->val);
            return &UNKNOWN_TYPE;
        }
        return struct_tmp->type;
    }
    // StructSpecifier -> STRUCT OptTag LC DefList RC
    else {
        Type_ptr type = (Type_ptr)malloc(sizeof(Type));
        type->kind = STRUCTURE;
        region_depth += 1;
        // Error[15]
        region_in_structure = 1;
        if (cur->child[3]) type->u.structure = DefList(cur->child[3]);
        region_in_structure = 0;
        compst_destroy(region_depth);
        region_depth -= 1;
        if (cur->child[1]) {
            Symbol_ptr prototype = new_symbol(region_depth);
            prototype->name = cur->child[1]->child[0]->val;
            prototype->is_proto = 1;
            prototype->type = type;
            // Error[16]
            if (hash_insert(prototype)) {
                semantic_error_option(16, cur->child[1]->lineno,
                                      "Duplicated name", prototype->name);
            }
        }
        return type;
    }
}

/*** Declarators ***/

Symbol_ptr FunDec(AST_node* cur, Type_ptr specifier_type, int is_claim) {
    Symbol_ptr tmp = new_symbol(region_depth);
    tmp->name = cur->child[0]->val;
    tmp->type = (Type_ptr)malloc(sizeof(Type));
    tmp->type->kind = FUNCTION;
    tmp->type->u.function.ret = specifier_type;
    tmp->type->u.function.is_claim = is_claim;
    // FunDec -> ID LP RP
    if (cur->child_num == 3) {
        tmp->type->u.function.params_num = 0;
        tmp->type->u.function.params = NULL;
    }
    // FunDec -> ID LP VarList RP
    else {
        tmp->type->u.function.params_num = 0;
        region_depth += 1;
        tmp->type->u.function.params = VarList(cur->child[2], tmp);
        if (is_claim) {
            compst_destroy(region_depth);
        }
        region_depth -= 1;
    }
    // Error[4], Error[19]
    Symbol_ptr already_func = hash_find(tmp->name, SEARCH_FUNCTION);
    if (already_func == NULL) {
        if (hash_insert(tmp)) {
            semantic_error_option(4, cur->child[0]->lineno,
                                  "Redefined function", tmp->name);
        }
        if (is_claim) {
            tmp->type->u.function.claim_lineno = cur->child[0]->lineno;
            claim_insert(tmp);
        }
    } else if (already_func->type->u.function.is_claim) {
        if (!equal_func(already_func->type, tmp->type)) {
            semantic_error(19, cur->child[0]->lineno,
                           "Function claim and defintion conflict");
        } else {
            if (!is_claim) already_func->type->u.function.is_claim = 0;
        }
    }

    return tmp;
}

Symbol_ptr VarList(AST_node* cur, Symbol_ptr func) {
    func->type->u.function.params_num++;
    Symbol_ptr tmp = ParamDec(cur->child[0]);
    // Error[3]
    if (hash_insert(tmp)) {
        semantic_error_option(3, cur->child[0]->lineno, "Redefined variable",
                              tmp->name);
    }
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
        tmp_type->u.array.elem = NULL;
        tmp_type->u.array.size = atoi(cur->child[2]->val);
        // int a[10][20][30] -> [10][20][30]a
        Type_ptr ite = tmp->type;
        if (ite->kind != ARRAY) {
            tmp_type->u.array.elem = ite;
            tmp->type = tmp_type;
        } else {
            while (ite->u.array.elem->kind == ARRAY) {
                ite = ite->u.array.elem;
            }
            tmp_type->u.array.elem = ite->u.array.elem;
            ite->u.array.elem = tmp_type;
        }
        return tmp;
    }
    // VarDec -> ID
    else {
        Symbol_ptr tmp = new_symbol(region_depth);
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
    // Error[3], Error[15]
    if (hash_insert(tmp)) {
        if (region_in_structure) {
            semantic_error_option(15, cur->child[0]->lineno, "Redefined field",
                                  tmp->name);
        } else {
            semantic_error_option(3, cur->child[0]->lineno,
                                  "Redefined variable", tmp->name);
        }
    }
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
    if (cur->child_num == 3) {
        Type_ptr type_exp = Exp(cur->child[2]);
        // Error[15]
        if (region_in_structure) {
            semantic_error(15, cur->child[1]->lineno,
                           "Assignment in structure definition");
        }
        // Error[5]
        else if (!equal_type(type_exp, tmp->type)) {
            semantic_error(5, cur->child[1]->lineno,
                           "Type mismatched for assignment");
        }
    }
    return tmp;
}

/*** Statments ***/

void CompSt(AST_node* cur) {
    region_depth += 1;
    // CompSt -> LC DefList StmtList RC
    if (cur->child[1]) {
        DefList(cur->child[1]);
    }
    if (cur->child[2]) {
        StmtList(cur->child[2]);
    }
    compst_destroy(region_depth);
    region_depth -= 1;
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
        // Error[8]
        Type_ptr type_ret = Exp(cur->child[1]);
        if (!equal_type(type_ret, region_func->type)) {
            semantic_error(8, cur->child[0]->lineno,
                           "Type mismatched for return");
        }
    }
    // Stmt -> WHILE LP Exp RP Stmt
    // Stmt -> IF LP Exp RP Stmt
    else if (cur->child_num == 5) {
        Type_ptr type_tmp = Exp(cur->child[2]);
        if (type_tmp->kind != BASIC) {
            semantic_error(7, cur->child[1]->lineno,
                           "Type mismatched for operands");
        }
        Stmt(cur->child[4]);
    }
    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    else if (cur->child_num == 7) {
        Type_ptr type_tmp = Exp(cur->child[2]);
        if (type_tmp->kind != BASIC) {
            semantic_error(7, cur->child[1]->lineno,
                           "Type mismatched for operands");
        }
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

Type_ptr Exp(AST_node* cur) {
    // ID LP Args RP
    // ID LP RP
    if (astcmp(1, LP)) {
        // Error[2], Error[11]
        if (!hash_find(cur->child[0]->val, SEARCH_FUNCTION)) {
            if (hash_find(cur->child[0]->val, SEARCH_VARIABLE)) {
                semantic_error_option(2, cur->child[0]->lineno,
                                      "Not a function", cur->child[0]->val);
            } else {
                semantic_error_option(2, cur->child[0]->lineno,
                                      "Undefined function", cur->child[0]->val);
            }
            return &UNKNOWN_TYPE;
        }

        Symbol_ptr type_func = hash_find(cur->child[0]->val, SEARCH_FUNCTION);
        // Error[9]
        int applicable = 1;
        if (type_func->type->u.function.params == NULL) {
            if (cur->child_num > 3) applicable = 0;
        } else {
            if (cur->child_num == 3)
                applicable = 0;
            else {
                AST_node* args = cur->child[2];
                Symbol_ptr func_args = type_func->type->u.function.params;
                while (args && func_args) {
                    Type_ptr type_arg = Exp(args->child[0]);
                    if (!equal_type(type_arg, func_args->type)) {
                        applicable = 0;
                        break;
                    }
                    if (args->child_num == 3) {
                        args = args->child[2];
                    } else {
                        args = NULL;
                    }
                    func_args = func_args->cross_nxt;
                }
                if (args || func_args) applicable = 0;
            }
        }

        if (!applicable) {
            semantic_error(9, cur->child[0]->lineno,
                           "Function not applicable for arguments");
            return &UNKNOWN_TYPE;
        }

        return type_func->type->u.function.ret;
    }
    // LP Exp RP
    else if (astcmp(0, LP)) {
        return Exp(cur->child[1]);
    }
    // Exp LB Exp RB
    else if (astcmp(1, LB)) {
        Type_ptr type_array = Exp(cur->child[0]);
        // Error[10]
        if (type_array->kind != ARRAY) {
            semantic_error(10, cur->child[0]->lineno, "Not an array");
            return &UNKNOWN_TYPE;
        }
        Type_ptr type_int = Exp(cur->child[2]);
        // Error[12]
        if (type_int->kind != BASIC || type_int->u.basic != INT) {
            semantic_error(12, cur->child[0]->lineno, "Not an integar");
            return &UNKNOWN_TYPE;
        }
        return type_array->u.array.elem;
    }
    // Exp DOT ID
    else if (astcmp(1, DOT)) {
        Type_ptr type_strcut = Exp(cur->child[0]);
        // Error[13]
        if (type_strcut->kind != STRUCTURE) {
            semantic_error(13, cur->child[0]->lineno, "Not a structure");
            return &UNKNOWN_TYPE;
        }
        Type_ptr ret = NULL;
        for (Symbol_ptr p = type_strcut->u.structure; p; p = p->cross_nxt) {
            if (strcmp(p->name, cur->child[2]->name) == 0) {
                ret = p->type;
                break;
            }
        }
        // Error[14]
        if (!ret) {
            semantic_error(14, cur->child[2]->lineno, "Illegal use of \".\"");
            return &UNKNOWN_TYPE;
        }
        return ret;
    }
    // Exp ASSIGNOP Exp
    else if (astcmp(1, ASSIGNOP)) {
        // Error[6]
        Type_ptr type_1 = Exp(cur->child[0]);
        if (!((cur->child[0]->child_num == 1 &&
               strcmp(cur->child[0]->child[0]->name, "ID") == 0) ||
              (cur->child[0]->child_num == 3 &&
               strcmp(cur->child[0]->child[1]->name, "DOT") == 0) ||
              (cur->child[0]->child_num == 4 &&
               strcmp(cur->child[0]->child[1]->name, "LB") == 0))) {
            semantic_error(
                6, cur->child[1]->lineno,
                "The left-hand side of an assignment must be a variable");
            return &UNKNOWN_TYPE;
        }
        Type_ptr type_2 = Exp(cur->child[2]);
        // Error[5]
        if (!equal_type(type_1, type_2)) {
            semantic_error(5, cur->child[1]->lineno,
                           "Type mismatched for assignment");
            return &UNKNOWN_TYPE;
        }
        return type_2;
    }
    // Exp AND Exp
    // Exp OR Exp
    // Exp RELOP Exp
    // Exp PLUS Exp
    // Exp MINUS Exp
    // Exp STAR Exp
    // Exp DIV Exp
    else if (cur->child_num == 3) {
        // Error[7]
        Type_ptr type_1 = Exp(cur->child[0]);
        Type_ptr type_2 = Exp(cur->child[2]);
        if (strcmp(cur->child[1]->name, "AND") == 0 ||
            strcmp(cur->child[1]->name, "OR") == 0 ||
            strcmp(cur->child[1]->name, "RELOP") == 0) {
            if ((type_1->kind != BASIC || type_1->u.basic != INT) ||
                (type_2->kind != BASIC || type_2->u.basic != INT)) {
                semantic_error(7, cur->child[1]->lineno,
                               "Type mismatched for operands");
                return &UNKNOWN_TYPE;
            }
        }
        if (type_1->kind != BASIC || type_2->kind != BASIC) {
            semantic_error(7, cur->child[1]->lineno,
                           "Type mismatched for operands");
            return &UNKNOWN_TYPE;
        }
        if (type_1->kind != BASIC || type_2->kind != BASIC) {
            semantic_error(7, cur->child[1]->lineno,
                           "Type mismatched for operands");
            return &UNKNOWN_TYPE;
        }
        if (!equal_type(type_1, type_2)) {
            semantic_error(7, cur->child[1]->lineno,
                           "Type mismatched for operands");
            return &UNKNOWN_TYPE;
        }

        return type_2;
    }
    // MINUS Exp
    // NOT Exp
    else if (cur->child_num == 2) {
        return Exp(cur->child[1]);
    }
    // ID
    else if (astcmp(0, ID)) {
        // Error[1]
        Symbol_ptr target = hash_find(cur->child[0]->val, SEARCH_VARIABLE);
        if (!target) {
            semantic_error_option(1, cur->child[0]->lineno,
                                  "Undefined variable", cur->child[0]->val);
            return &UNKNOWN_TYPE;
        }
        return target->type;
    }
    // INT
    else if (astcmp(0, INT)) {
        return &INT_TYPE;
    }
    // FLOAT
    else if (astcmp(0, FLOAT)) {
        return &FLOAT_TYPE;
    }
    return &UNKNOWN_TYPE;
}

/*--------------------------------------------------------------------
 * semantic.c
 *------------------------------------------------------------------*/
