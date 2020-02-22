#include "stdio.h"
#ifdef DEBUG
    extern int yydebug;
#endif

void yyrestart (FILE *input_file  );
int yyparse (void);

int error_num = 0;
int throw_error(char type, int lineno, char* msg){
    error_num++;
    fprintf(stderr, "Error type %c at Line %d: %s\n", type, lineno, msg);
    return 0;
}

int main(int argc, char **argv){
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
#ifdef DEBUG
    yydebug = 1;
#endif
    yyparse();
    return 0;
}