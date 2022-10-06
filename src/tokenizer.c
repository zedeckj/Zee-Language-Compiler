//
// Created by Jordan on 7/11/22.
//

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define SPECIAL_COUNT 8
char special[SPECIAL_COUNT] = {'(',')',';', ',', ':', '#', '{', '}'}; //'0', '1', '2', '3','4','5','6','7','8', '9'};


int special_index(char c) {
    for (int i = 0; i < SPECIAL_COUNT; i++) {
        if (c == special[i]) return i;
    }
    return -1;
}

int isempty(char c) {
    return isspace(c) || c == '\0';
}


char ** tokenize(char * str_in, char ** tokens, int * token_count, int * locations) {
    *token_count = 0;
    int len = strlen(str_in);
    int size;
    int line = 1;
    int quotes = 0;
    char temp_token[100] = {'\0'};
    for (int j = 0, i = 0; i < len + 1; j++, i++) {
        int si = special_index(str_in[i]);
        if (str_in[i] == '\n') {
            line++;
        }
        else if (str_in[i] == '\"') {
            quotes++;
        }
        else if (str_in[i] == '\'') {
            quotes+=3;
        }
        if (quotes == 2 || quotes == 6) {
            temp_token[j] = (quotes == 2 ? '\"' : '\'');
            temp_token[j+1] = '\0';
            tokens[*token_count] = malloc(sizeof(char) * (j+2));
            locations[*token_count] = line;
            strncpy(tokens[*token_count], temp_token, j+2);
            *token_count += 1;
            quotes = 0;
            j = -1;

        }
        else if (!quotes && (isempty(str_in[i]) || si > -1)) {
            if (j != 0) {
                temp_token[j] = '\0';
                tokens[*token_count] = malloc(sizeof(char) * (j+1));
                locations[*token_count] = line;
                strncpy(tokens[*token_count], temp_token, j + 1);
                *token_count += 1;
            }
            if (si > -1) {
                tokens[*token_count] = malloc(sizeof(char));
                tokens[*token_count][0] = str_in[i];
                locations[*token_count] = line;
                *token_count += 1;
            }
            j = -1;
        }
        else {
            temp_token[j] = str_in[i];
        }


    }
    return tokens;
}
