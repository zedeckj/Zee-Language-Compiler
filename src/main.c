#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "scanner.h"
#include "compiler.h"
#include "linearizer.h"


#define MAX_TOKENS 1000
#define MAX_LINES 1000

void print_tokens(char ** tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        printf("token: %s\n", tokens[i]);
        /*for (int j = 0; j < strlen(tokens[i]); j++) {
            printf("%d ", tokens[i][j]);
        }
        printf("\n");
         */
    }
}


int main(int argv, char ** args) {
    assert(argv > 1);
    char * filename = args[1];
    int dlt_asm = argv < 3 || !(args[2][0] == '-' && args[2][1] == 'S');
    char correct_argument_count = (argv < 4);
    char is_zee_filetype = strcmp(filename + strlen(filename) - 3, "zee") == 0;
    assert(correct_argument_count);
    assert(is_zee_filetype);


    char * str_in = malloc(sizeof(char) * 5000);
    int * token_count = malloc(sizeof(int));
    int * line_count = malloc(sizeof(int));
    char ** tokens = malloc(sizeof(char *) * MAX_TOKENS);
    int * locations = malloc(sizeof(int) * MAX_TOKENS);
    struct line * lines = malloc(sizeof(struct line) * MAX_LINES);
    scan(filename, str_in);
    //printf("\n\n%s\n\n", str_in);
    tokenize(str_in, tokens, token_count, locations);
    //print_tokens(tokens, *token_count);
    if (validate_program(tokens, locations, *token_count, filename, 0)) {
        filename[strlen(filename) - 4] = '\0';
        //printf("validated\n");
        char cmpl[100];
        sprintf(cmpl, "%s.s", filename);
        compile_program(tokens, *token_count, cmpl);
        char as_cmd[100];
        sprintf(as_cmd, "as %s -o %s.o", cmpl, filename);
        system(as_cmd);
        //system("as test.s -o test.o");
        char ld_cmd[100];
        sprintf(ld_cmd, "ld %s.o -o %s -lSystem", filename, filename);
        //system("ld test.o -o exec -lSystem");
        system(ld_cmd);
        system("rm -r *.o");
        if (dlt_asm) system("rm -r *.s");
    }

    //print_lines(lines, *line_count);
    free(str_in);
    for (int i = 0; i < *token_count; i++) free(tokens[i]);
    free(token_count);
    free(tokens);
    for (int i = 0; i < *line_count; i++){
        free(lines[i].tokens);
        free(lines[i].locations);
    }
    free(lines);
    free(line_count);
    ;;



    return 0;
}
