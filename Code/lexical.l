%option yylineno

%{
void error();
int number(char *, int);
float floatvalue(char *, int);
%}

relop >|<|>=|<=|<=|==|!=
type int|float

whitespace [ \t\r\a\n]+
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

{whitespace} {}

"struct" { printf("STRUCT\n"); }
"return" { printf("RETURN\n"); }
"if" { printf("IF\n"); }
"else" { printf("ELSE\n"); }
"while" { printf("WHILE\n"); }
{type} { printf("TYPE: %s\n", yytext); }

{relop} { printf("RELOP\n"); }
";" { printf("SEMI\n"); }
"," { printf("COMMA\n"); }
"=" { printf("ASSIGNOP\n"); }
"+" { printf("PLUS\n"); }
"-" { printf("MINUS\n"); }
"*" { printf("STAR\n"); }
"/" { printf("DIV\n"); }
"&&" { printf("AND\n"); }
"||" { printf("OR\n"); }
"." { printf("DOT\n"); }
"!" { printf("NOT\n"); }
"(" { printf("LP\n"); }
")" { printf("RP\n"); }
"[" { printf("LB\n"); }
"]" { printf("RB\n"); }
"{" { printf("LC\n"); }
"}" { printf("RC\n"); }

{err_int8}  { error("Illegal octal number"); }
{err_int16} { error("Illegal hexadecimal number"); }
{err_float} { error("Illegal float point number"); }
{err_sci}   { error("Illegal float point number"); }

{int10} { printf("INT: %d\n", number(yytext, 10)); }
{int8}  { printf("INT: %d\n", number(yytext, 8)); }
{int16} { printf("INT: %d\n", number(yytext, 16)); }
{float} { printf("FLOAT: %f\n", floatvalue(yytext, 0)); }
{sci} { printf("FLOAT: %f\n", floatvalue(yytext, 1)); }

{err_id} { error("Illegal id"); }
{id} { printf("ID: %s\n", yytext); }

. { error("Mysterious characters"); }

%%

void error(const char* msg){
    printf("Error type A at Line %d: %s \'%s\'\n", yylineno, msg, yytext);
}

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