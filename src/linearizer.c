//
// Created by Jordan on 7/11/22.
//

#include <stdlib.h>
#include <printf.h>
#include "linearizer.h"

char * keywords [] = {"set", "add", "mul", "div", "goto", "goto-if", "label", "putn", "printf", "I64"};

void linearize(char ** tokens, int token_count, struct line * lines, int * line_count, int * locations) {
    int l = 0;
    *line_count = 0;
    int semicolon = 0;
    for (int i = 0, size = 0, start = 0; i < token_count; i++, size++) {
        if (semicolon) {
            char ** subtokens = malloc(sizeof(char *) * size);
            int * sublocs = malloc(sizeof(int) * size);
            for (int j = 0; j < size; j++) {
                subtokens[j] = tokens[start + j];
                sublocs[j] = locations[start + j];
            }
            lines[l].token_count = size;
            lines[l].tokens = subtokens;
            lines[l].locations = sublocs;
            start = i;
            size = 0;
            l++;
            (*line_count)++;
        }
        semicolon = tokens[i][0] == ';' || tokens[i][0] == ':';
    }
    //printf("\n\n" );
}
