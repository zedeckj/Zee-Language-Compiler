//
// Created by Jordan on 7/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void scan(char * filename, char * out_str) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "file: \"%s\" does not exist\n", filename);
        return;
    }
    //out_str[0] = '\0';
    char temp[5000];
    int space = 5000;
    int i = 0;
    while (fgets(temp, space, file)) {
        int len = strlen(temp) + 1;
        strcat(out_str, temp);
        space -= len;
    }
}