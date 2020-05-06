%{
/*
 * File: syntax.y
 * Project: C--_Compiler
 * File Created: 2020-03-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

/*--------------------------------------------------------------------
 *
 * Includes
 *
 *------------------------------------------------------------------*/
#include "common.h"
#include "syntax.h"
#include "error.h"
#include "ast.h"
#include "lex.yy.c"

#define generate(token, self1, self2, childnum, ...) \
    self1 = ast_create(#token, "", "", self2.first_line); \
    ast_append(self1, childnum, __VA_ARGS__)
%}
%locations

/*--------------------------------------------------------------------
 *
 * definitions
 *
 *------------------------------------------------------------------*/
%union { AST_node* node; }

/*--------------------------------------------------------------------
 *
 * terminal-symbols
 *
 *------------------------------------------------------------------*/
%token    <node> INT FLOAT ID SEMI LC RC COMMA TYPE STRUCT RETURN IF WHILE
%nonassoc <node> LOWER_THAN_ELSE
%nonassoc <node> ELSE

/* operator precedence */
%right     <node> ASSIGNOP        /* priority 8 */
%left     <node> OR              /* priority 7 */
%left     <node> AND             /* priority 6 */
%left     <node> RELOP           /* priority 5 */
%left     <node> PLUS MINUS      /* priority 4 */
%left     <node> STAR DIV        /* priority 3 */
%right    <node> NOT NEG         /* priority 2 */
%left     <node> LP RP LB RB DOT /* priority 1 */

/*--------------------------------------------------------------------
 *
 * nonterminal-symbols
 *
 *------------------------------------------------------------------*/
%type    <node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

/*--------------------------------------------------------------------
 *
 * start of grammer
 *
 *------------------------------------------------------------------*/
%start   Program

%%

/*--------------------------------------------------------------------
 *
 * rules
 *
 *------------------------------------------------------------------*/
/* High-Level Definitions */
Program
    : ExtDefList
      {
        if (!$$){
          @$.first_line = yylineno;
        }
        generate(Program,$$,@$,1,$1); ast_root = $$;
      }
    ;
ExtDefList
    : /* empty */
      { $$ = NULL; }
    | ExtDef ExtDefList
      { generate(ExtDefList,$$,@$,2,$1,$2); }
    ;
ExtDef
    : Specifier ExtDecList SEMI
      { generate(ExtDef,$$,@$,3,$1,$2,$3); }
    | Specifier ExtDecList
      { errorB("Missing ';'"); }
    | Specifier ExtDecList error SEMI
      { errorB("something wrong in global definition"); }
    | Specifier error SEMI
      { errorB("something wrong in global definition"); }
    | Specifier SEMI
      { generate(ExtDef,$$,@$,2,$1,$2); }
    | Specifier error
      { errorB("Missing ';'"); }
    | Specifier FunDec CompSt
      { generate(ExtDef,$$,@$,3,$1,$2,$3); }
		| Specifier FunDec SEMI
			{ generate(ExtDef,$$,@$,3,$1,$2,$3); }
    | Specifier error CompSt
      { errorB("error in function definition"); }
    ;
ExtDecList
    : VarDec
      { generate(ExtDecList,$$,@$,1,$1); }
    | VarDec COMMA ExtDecList
      { generate(ExtDecList,$$,@$,3,$1,$2,$3); }
    | VarDec COMMA error
      { errorB("something wrong in global definition"); }
    | error COMMA ExtDecList
      { errorB("something wrong in global definition"); }
    ;

/* Specifier */
Specifier
    : TYPE
      { generate(Specifier,$$,@$,1,$1); }
    | StructSpecifier
      { generate(Specifier,$$,@$,1,$1); }
    ;
StructSpecifier
    : STRUCT OptTag LC DefList RC
      { generate(StructSpecifier,$$,@$,5,$1,$2,$3,$4,$5); }
    | STRUCT OptTag LC DefList error RC
      { errorB("something wrong in the definition of structure");}
    | STRUCT Tag
      { generate(StructSpecifier,$$,@$,2,$1,$2); }
    ;
OptTag
    : /* empty */
      { $$ = NULL; }
    | error
      { errorB("something wrong in struct tag"); }
    | ID
      { generate(OptTag,$$,@$,1,$1); }
    ;
Tag
    : ID
      { generate(Tag,$$,@$,1,$1); }
    ;

/* Declarators */
VarDec
    : ID
      { generate(VarDec,$$,@$,1,$1); }
    | VarDec LB INT RB
      { generate(VarDec,$$,@$,4,$1,$2,$3,$4); }
    | VarDec LB INT error RB
      { errorB("missing \"]\""); }
    | VarDec LB error
      { errorB("missing \"]\""); }
    | VarDec LB INT COMMA INT RB
      { errorB("expression error"); }
    ;
FunDec
    : ID LP VarList RP
      { generate(FunDec,$$,@$,4,$1,$2,$3,$4); }
    | ID LP VarList error RP
      { errorB("something wrong in the definition of structure"); }
    | ID LP VarList error LC
      { errorB("something wrong in the definition of structure"); }
    | ID LP RP
      { generate(FunDec,$$,@$,3,$1,$2,$3); }
    | ID LP error LC
      { errorB("something wrong in the definition of structure"); }
    ;
VarList
    : ParamDec COMMA VarList
      { generate(VarList,$$,@$,3,$1,$2,$3); }
    | error COMMA
      { errorB("something wrong before \",\""); }
    | ParamDec
      { generate(VarList,$$,@$,1,$1); }
    ;
ParamDec
    : Specifier VarDec
      { generate(ParamDec,$$,@$,2,$1,$2); }
    ;

/* Statements */
CompSt
    : LC DefList StmtList RC
      { generate(CompSt,$$,@$,4,$1,$2,$3,$4); }
    | error RC
      { errorB("wrong statements in block"); }
    ;
StmtList
    : /* empty */
      { $$ = NULL; }
    | Stmt StmtList
      { generate(StmtList,$$,@$,2,$1,$2); }
    | Stmt error SEMI StmtList
      { errorB("statement error"); }
    ;
Stmt
    : Exp SEMI
      { generate(Stmt,$$,@$,2,$1,$2); }
    | Exp error SEMI
      { errorB("expression error"); }
    | CompSt
      { generate(Stmt,$$,@$,1,$1); }
    | RETURN Exp SEMI
      { generate(Stmt,$$,@$,3,$1,$2,$3); }
    | RETURN Exp error SEMI
      { errorB("return error"); }
    | RETURN Exp error
      { errorB("Missing ';'"); }
    | IF LP Exp RP Stmt
      %prec LOWER_THAN_ELSE
      { generate(Stmt,$$,@$,5,$1,$2,$3,$4,$5); }
    | IF LP error Stmt
      { errorB("missing \")\""); }
    | IF LP Exp RP Stmt ELSE Stmt
      { generate(Stmt,$$,@$,7,$1,$2,$3,$4,$5,$6,$7); }
    | IF LP Exp RP error ELSE Stmt
      { errorB("missing \";\""); }
    | WHILE LP Exp RP Stmt
      { generate(Stmt,$$,@$,5,$1,$2,$3,$4,$5); }
    | WHILE LP error Stmt
      { errorB("missing \")\""); }

/* Local Definitions */
DefList
    : /* empty */
      { $$ = NULL; }
    | Def DefList
      { generate(DefList,$$,@$,2,$1,$2); }
    ;
Def
    : Specifier DecList SEMI
      { generate(Def,$$,@$,3,$1,$2,$3); }
    | Specifier DecList error SEMI
      { errorB("something wrong in local definition"); }
    | Specifier DecList error
      { errorB("missing ';'"); }
    | Specifier error
      { errorB("Missing ';'"); }
    | Specifier error SEMI
      { errorB("something wrong in local definition"); }
    ;
DecList
    : Dec
      { generate(DecList,$$,@$,1,$1); }
    | Dec COMMA DecList
      { generate(DecList,$$,@$,3,$1,$2,$3); }
    | error COMMA DecList
      { errorB("something wrong in definitions"); }
    ;
Dec
    : VarDec
      { generate(Dec,$$,@$,1,$1); }
    | VarDec ASSIGNOP Exp
      { generate(Dec,$$,@$,3,$1,$2,$3); }
    | VarDec ASSIGNOP
      { errorB("initial value not given"); }
    ;

/* Expression */
Exp
    : Exp ASSIGNOP Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp ASSIGNOP error
      { errorB("expression error with '='"); }
    | Exp AND Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp AND error
      { errorB("expression error with '&'"); }
    | Exp OR Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp OR error
      { errorB("expression error with '|'"); }
    | Exp RELOP Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp RELOP error
      { errorB("expression error"); }
    | Exp PLUS Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp PLUS error
      { errorB("expression error with '+'"); }
    | Exp MINUS Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp MINUS error
      { errorB("expression error with '-'"); }
    | Exp STAR Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp STAR error
      { errorB("expression error with '*'"); }
    | Exp DIV Exp
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | Exp DIV error
      { errorB("expression error with '/'"); }
    | LP Exp RP
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | LP Exp error RP
      { errorB("expression error in parentheses"); }
    | MINUS Exp %prec NEG
      { generate(Exp,$$,@$,2,$1,$2); }
    | NOT Exp
      { generate(Exp,$$,@$,2,$1,$2); }
    | ID LP Args RP
      { generate(Exp,$$,@$,4,$1,$2,$3,$4); }
    | ID LP Args error RP
      { errorB("expression error in function"); }
    | ID LP RP
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | ID LP error RP
      { errorB("expression error in function"); }
    | Exp LB Exp RB
      { generate(Exp,$$,@$,4,$1,$2,$3,$4); }
    | Exp LB Exp error RB
      { errorB("missing \"]\""); }
    | Exp LB RB
      { errorB("in '[]'"); }
    | Exp LB Exp COMMA Exp RB
      { errorB("expression error"); }
    | Exp DOT ID
      { generate(Exp,$$,@$,3,$1,$2,$3); }
    | DOT ID
      { errorB("expression error"); }
    | ID
      { generate(Exp,$$,@$,1,$1); }
    | INT
      { generate(Exp,$$,@$,1,$1); }
    | FLOAT
      { generate(Exp,$$,@$,1,$1); }
    ;
Args
    : Exp COMMA Args
      { generate(Args,$$,@$,3,$1,$2,$3); }
    | Exp
      { generate(Args,$$,@$,1,$1); }
    ;

%%
/*------------------------------------------------------------------------------
 *
 * functions
 *
 *----------------------------------------------------------------------------*/
int yyerror(char* msg){
  errorF();
	return 0;
}

/*--------------------------------------------------------------------
 * syntax.y
 *------------------------------------------------------------------*/
