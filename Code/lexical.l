/**
 * C-- Compiler, Labs of NJU course Principles and Techiques of Compiler
 *
 * Author: Zangwei Zheng, zhengzangw@163.com
 *
 **/
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
#include <string.h>

/*--------------------------------------------------------------------
 * 
 * global variables
 * 
 *------------------------------------------------------------------*/
int yycolumn = 1;

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

{newline}                       { yycolumn = 1; newline_error = 1; }
{whitespace}                    {}
"//"                            { comment_line(); }
"/*"                            { int status = comment_block();
                                  if (status == 1) errorA("unterminated comment");
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

{int10}                         { handles(INT, number(yytext, 10)); }
{int8}                          { handles(INT, number(yytext, 8)); }
{int16}                         { handles(INT, number(yytext, 16)); }

{float}                         { handles(FLOAT, floatvalue(yytext, 0)); }
{sci}                           { handles(FLOAT, floatvalue(yytext, 1)); }

{err_id}                        { errorA("Illegal id"); }
{id}                            { handle(ID); }

.                               { errorA("Mysterious characters"); }

%%

char* number(char *digits, int base){
    int len = strlen(digits);
    int ans = 0;
    int i = 0;
    if (base == 16) i = 2;
    for (;i<len;++i){
        char c = digits[i];
        if (c>='0' && c<='9')
            ans = ans * base + c - '0';
        else if (c>='a' && c<='f')
            ans = ans * base + c - 'a' + 10;
        else
            ans = ans * base + c - 'A' + 10;
    }
    char* ret = (char *)malloc(32);
    sprintf(ret, "%d", ans);
    handle(ret);
}

char* floatvalue(char *digits, int type){
    float ans = 0;
    int len = strlen(digits);
    int part1 = 0;
    float part2 = 0;
    int i = 0;
    for (;i<len;++i){
        char c = digits[i];
        if (c != '.') part1 = part1*10 + c - '0';
        else break;
    }
    i++;
    float base = 1;
    for (;i<len && digits[i]!='E' && digits[i]!='e';++i){
        char c = digits[i];
        base /= 10;
        part2 += (c - '0') * base;
    }
    ans = (float)part1 + part2;
    
    if (type==1){
        int sgn = (digits[i+1]=='-') ? -1 : 1;
        if (digits[i+1] == '+' || digits[i+1] == '-') {
            i+=2;
        }
        else {
            i++;
        }
        int index = atoi(digits+i);
        for (int j=0;j<index;++j) 
            ans = (sgn == 1) ? ans*10 : ans/10;
    }

    char* ret = (char *)malloc(32);
    sprintf(ret, "%f", ans);
    handle(ret);
}

int comment_line(){
    char c = input();
    while (c != '\n' && c != EOF) c = input();
    return 0;
}

int comment_block(){
    char c1 = input(), c2;
    if (c1 != EOF) c2 = input(); else return 1;
    while (c2 != EOF){
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