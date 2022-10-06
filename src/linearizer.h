//
// Created by Jordan on 7/11/22.
//

#ifndef NONFUNCTIONAL_LISP_COMPILER_LINEARIZER_H
#define NONFUNCTIONAL_LISP_COMPILER_LINEARIZER_H

struct line {
    char ** tokens;
    int token_count;
    int * locations;
};

void linearize(char ** tokens, int token_count, struct line * lines, int * line_count, int * locations);

#endif //NONFUNCTIONAL_LISP_COMPILER_LINEARIZER_H
