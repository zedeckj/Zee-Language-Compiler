//
// Created by Jordan on 7/15/22.
//

#ifndef NONFUNCTIONAL_LISP_COMPILER_COMPILER_H
#define NONFUNCTIONAL_LISP_COMPILER_COMPILER_H

int validate_program(char ** tokens, int * locations, int token_count, char * filename, int print_all);
int compile_program(char ** tokens, int token_count, char * outname);

#endif //NONFUNCTIONAL_LISP_COMPILER_COMPILER_H
