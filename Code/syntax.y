%locations
%{
    #include "lex.yy.c"
        #define lex_throw_error(msg) throw_error('B', yylineno, msg)
    int yyerror(char* msg);
    int throw_error();
%}

%token INT
%token FLOAT
%token ID
%token SEMI LC RC COMMA
%token TYPE STRUCT RETURN IF WHILE
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left  LP RP LB RB DOT /* priority 1 */
%right NOT             /* priority 2 */
%left  STAR DIV        /* priority 3 */
%left  PLUS MINUS      /* priority 4 */
%left  RELOP           /* priority 5 */
%left  AND             /* priority 6 */
%left  OR              /* priority 7 */
%left  ASSIGNOP        /* priority 8 */

%start Program

%%

/* High-Level Definitions */
Program         : ExtDefList
                ;
ExtDefList      : /* empty */
                | ExtDef ExtDefList
                ;
ExtDef          : Specifier ExtDecList SEMI
                | Specifier SEMI
                | Specifier FunDec CompSt
                ;
ExtDecList      : VarDec
                | VarDec COMMA ExtDecList
                ;

/* Specifier */
Specifier       : TYPE
                | StructSpecifier
                ;
StructSpecifier : STRUCT OptTag LC DefList RC
                | STRUCT Tag
                ;
OptTag          : /* empty */
                | ID
                ;
Tag             : ID
                ;

/* Declarators */
VarDec          : ID
                | VarDec LB INT RB
                ;
FunDec          : ID LP VarList RP
                | ID LP RP
                ;
VarList         : ParamDec COMMA VarList
                | ParamDec
                ;
ParamDec        : Specifier VarDec
                ;

/* Statements */
CompSt          : LC DefList StmtList RC
                ;
StmtList        : /* empty */
                | Stmt StmtList
                ;
Stmt            : Exp SEMI
                | CompSt
                | RETURN Exp SEMI
                | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
                | IF LP Exp RP Stmt ELSE Stmt
                | WHILE LP Exp RP Stmt

/* Local Definitions */
DefList         : /* empty */
                | Def DefList
                ;
Def             : Specifier DecList SEMI
                ;
DecList         : Dec
                | Dec COMMA DecList
                ;
Dec             : VarDec
                | VarDec ASSIGNOP Exp
                ;

/* Expression */
Exp             : Exp ASSIGNOP Exp
                | Exp AND Exp
                | Exp OR Exp
                | Exp RELOP Exp
                | Exp PLUS Exp
                | Exp MINUS Exp
                | Exp STAR Exp
                | Exp DIV Exp
                | LP Exp RP
                | MINUS Exp
                | NOT Exp
                | ID LP Args RP
                | ID LP RP
                | Exp LB Exp RB
                | Exp DOT ID
                | ID
                | INT
                | FLOAT
                ;
Args            : Exp COMMA Args
                | Exp
                ;

%%

int yyerror(char* msg){
    /* empty */
}