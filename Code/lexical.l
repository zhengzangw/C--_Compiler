/*
 * File: lexical.l
 * Project: C--_Compiler
 * File Created: 2020-03-22
 * Author: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Last Modified: 2020-03-29
 * Modified By: Zangwei Zheng (zzw@smail.nju.edu.cn)
 * -----
 * Copyright 2020 NJU, Zangwei Zheng
 */

%option yylineno

%{
/*--------------------------------------------------------------------
 *
 * Includes
 *
 *------------------------------------------------------------------*/
#include "common.h"
#include "error.h"
#include "ast.h"
#include "lexical.h"
#include "syntax.tab.h"
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------
 *
 * global variables
 *
 *------------------------------------------------------------------*/
int yycolumn = 1;
char* number(char *digits);
char* floatvalue(char *digits);

/*--------------------------------------------------------------------
 *
 * defines
 *
 *------------------------------------------------------------------*/
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
#define handle(type) yylval.node = ast_create(#type, yytext, yytext, yylineno); return (type)
#define handles(type, val) yylval.node = ast_create(#type, yytext, val, yylineno); return (type)
%}

/*--------------------------------------------------------------------
 *
 * flex definition
 *
 *------------------------------------------------------------------*/
/* general */
relop                           >|<|>=|<=|<=|==|!=
type                            int|float
newline                         \n
whitespace                      [ \t\r\a]+
digit                           [0-9]
letter                          [_a-zA-Z]
id                              {letter}[_a-zA-Z0-9]*
/* INT */
int10                           0|[1-9]{digit}*
int8                            0[0-7]+
int16                           (0x|0X)[0-9A-Fa-f]+
/* FLOAT */
float                           {digit}+\.{digit}+
sci_index                       [+-]?{digit}+
sci_normal                      {digit}+\.{digit}+[Ee]{sci_index}
sci_no_before                   \.{digit}+[Ee]{sci_index}
sci_no_after                    {digit}+\.[Ee]{sci_index}
sci                             ({sci_normal}|{sci_no_after}|{sci_no_before})
/* ERROR */
err_int8                        0{digit}*[8-9]{digit}*
err_int16                       (0x|0X)[0-9A-Fa-f]*[g-zG-Z][0-9A-Fa-f]*
err_float                       (\.{digit}+|{digit}+\.)
err_sci                         ({digit}+\.{digit}*[Ee]|{digit}*\.{digit}+[Ee])
err_id                          {digit}+{id}

%%

<<EOF>>                         {
                                    print_error();
                                    yycolumn = newline_error = 1;
                                    return 0;
                                }
{newline}                       {
                                    print_error();
                                    yycolumn = newline_error = 1;
                                }
{whitespace}                    {}
"//"                            { comment_line(); }
"/*"                            {
                                  int status = comment_block();
                                  if (status == 1) errorA("Unterminated comment");
                                }
"struct"                        { handle(STRUCT); }
"return"                        { handle(RETURN); }
"if"                            { handle(IF); }
"else"                          { handle(ELSE); }
"while"                         { handle(WHILE); }
{type}                          { handle(TYPE); }
{relop}                         { handle(RELOP); }
";"                             { handle(SEMI); }
","                             { handle(COMMA); }
"="                             { handle(ASSIGNOP); }
"+"                             { handle(PLUS); }
"-"                             { handle(MINUS); }
"*"                             { handle(STAR); }
"/"                             { handle(DIV); }
"&&"                            { handle(AND); }
"||"                            { handle(OR); }
"."                             { handle(DOT); }
"!"                             { handle(NOT); }
"("                             { handle(LP); }
")"                             { handle(RP); }
"["                             { handle(LB); }
"]"                             { handle(RB); }
"{"                             { handle(LC); }
"}"                             { handle(RC); }

{err_int8}                      { errorA("Illegal octal number"); }
{err_int16}                     { errorA("Illegal hexadecimal number"); }
{err_float}                     { errorA("Illegal float point number"); }
{err_sci}                       { errorA("Illegal float point number"); }

{int10}                         { handles(INT, number(yytext)); }
{int8}                          { handles(INT, number(yytext)); }
{int16}                         { handles(INT, number(yytext)); }

{float}                         { handles(FLOAT, floatvalue(yytext)); }
{sci}                           { handles(FLOAT, floatvalue(yytext)); }

{err_id}                        { errorA("Illegal id"); }
{id}                            { handle(ID); }

.                               { errorA("Mysterious characters"); }

%%

char* number(char *digits){
    unsigned int ans = (unsigned int)strtoul(digits, NULL, 0);
    char* ret = (char *)malloc(128);
    sprintf(ret, "%u", ans);
    return ret;
}

char* floatvalue(char *digits){
    float ans = (float)atof(digits);
    char* ret = (char *)malloc(128);
    sprintf(ret, "%f", ans);
    return ret;
}

int comment_line(){
    char c = input();
    while (c != '\n' && c != EOF) c = input();
    return 0;
}

int comment_block(){
    char c1 = input(), c2;
    if (c1 != '\0') c2 = input(); else return 1;
    while (c2 != '\0'){
        if (c1 == '*' && c2 == '/') return 0;
        char t = input();
        c1 = c2;
        c2 = t;
    }
    return 1;
}

/*--------------------------------------------------------------------
 * lexical.l
 *------------------------------------------------------------------*/
