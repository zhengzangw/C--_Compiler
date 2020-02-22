%option yylineno

%{
    #include "syntax.tab.h"
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
    #define lex_throw_error(msg) throw_error('A', yylineno, msg)

    int throw_error();
    int number(char *, int);
    float floatvalue(char *, int);
    int yycolumn = 1;
%}


relop >|<|>=|<=|<=|==|!=
type int|float
newline \n
whitespace [ \t\r\a]+
digit [0-9]
letter [_a-zA-Z]
id {letter}[_a-zA-Z0-9]*

int10 0|[1-9]{digit}*
int8 0[0-7]+
int16 (0x|0X)[0-9A-Fa-f]+

float {digit}+\.{digit}+
sci_index [+-]?{digit}+
sci_normal {digit}+\.{digit}+[Ee]{sci_index}
sci_no_before \.{digit}+[Ee]{sci_index}
sci_no_after  {digit}+\.[Ee]{sci_index}
sci ({sci_normal}|{sci_no_after}|{sci_no_before})

err_int8 0{digit}*[8-9]{digit}*
err_int16 (0x|0X)[0-9A-Fa-f]*[g-zG-Z][0-9A-Fa-f]*
err_float (\.{digit}+|{digit}+\.)
err_sci ({digit}+\.{digit}*[Ee]|{digit}*\.{digit}+[Ee])
err_id {digit}+{id}

%%

{newline} { yycolumn = 1; }
{whitespace} {}
"struct" { return STRUCT; }
"return" { return RETURN; }
"if" { return IF; }
"else" { return ELSE; }
"while" { return WHILE; }
{type} { 
    //strcpy(yylval, yytext);
    return TYPE;
}
{relop} { return RELOP; }
";" { return SEMI; }
"," { return COMMA; }
"=" { return ASSIGNOP; }
"+" { return PLUS; }
"-" { return MINUS; }
"*" { return STAR; }
"/" { return DIV; }
"&&" { return AND; }
"||" { return OR; }
"." { return DOT; }
"!" { return NOT; }
"(" { return LP; }
")" { return RP; }
"[" { return LB; }
"]" { return RB; }
"{" { return LC; }
"}" { return RC; }

{err_int8}  { lex_throw_error("Illegal octal number"); }
{err_int16} { lex_throw_error("Illegal hexadecimal number"); }
{err_float} { lex_throw_error("Illegal float point number"); }
{err_sci}   { lex_throw_error("Illegal float point number"); }

{int10} { 
    yylval = number(yytext, 10);
    return INT;
}
{int8}  { 
    yylval = number(yytext, 8);
    return INT;
}
{int16} { 
    yylval = number(yytext, 16);
    return INT;
}

{float} { 
    yylval = floatvalue(yytext, 0);
    return FLOAT;
}
{sci} { 
    yylval = floatvalue(yytext, 1);
    return FLOAT;
}

{err_id} { lex_throw_error("Illegal id"); }
{id} {
    //strcpy(yylval, yytext);
    return ID;
}

. { lex_throw_error("Mysterious characters"); }

%%

int number(char *digits, int base){
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
    return ans;
}

float floatvalue(char *digits, int type){
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
        int index = number(digits+i, 10);
        for (int j=0;j<index;++j) 
            ans = (sgn == 1) ? ans*10 : ans/10;
    }

    return ans;
}