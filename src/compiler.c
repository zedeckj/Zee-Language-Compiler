//
// Created by Jordan on 7/12/22.
//

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "compiler.h"

char * variables[100];
int vari = 0;
char * labels[100];
int labi = 0;
char * filename;
char * strings[100];
int stri = 0;
FILE * out;
int sections[100];
int label_locations[100];
int lls = 0;
int current_numeric_label = 0;
int c = 0;
int seci;
int continues = 0;
int maxline;


int is_number(char * str);
int valid_expression(char ** tokens, int count);
int add_numeric_label(int gotoline, int currentline, int * locations);

int streql(char * str1, char * str2) {
    return strcmp(str1,str2) == 0;
}

int is_modifier(char * str) {
    char * modifiers[] = {"=", "+=", "*=", "/=", "-="};
    for (int i = 0; i < 5; i++) {
        if (streql(str, modifiers[i])) return 1;
    }
    return 0;
}

int is_variable(char * token) {
    for (int i = 0; i < vari; i++) {
        if (streql(token, variables[i])) return 1;
    }
    return 0;
}

int is_register(char * arg) {
    char * registers [18] = {"%rax",  "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%rsp", "%rbp", "%rsp", "%rsp", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
    for (int i = 0; i < 18; i++) {
        if (streql(arg,registers[i])) return 1;
    }
    if (arg[0] == '%') {
        fprintf(stderr, "%s is not a valid register, this message should not be seen.", arg);
    }
    return 0;

}

char * get_arg_register(int i) {
    switch (i){
        case 0:
            return "%rdi";
        case 1:
            return "%rsi";
        case 2:
            return "%rdx";
        case 3:
            return "%rcx";
        case 4:
            return "%r8";
        case 5:
            return "%r9";
        default:
            return NULL;

    }
}

char * get_operator(char ** tokens, int count) {
    if (tokens[0][0] == '{') {
        return tokens[1];
    }
    return tokens[0];
}

int is_operator(char ** tokens, int count) {
    int offset = 0;
    if (tokens[0][0] == '{') {
        offset = 1;
        if (count < 4 || !(is_variable(tokens[2]) || is_number(tokens[2]) || (tokens[2][0] == '(' && valid_expression(tokens + 2, count - 2)) ||  tokens[3][0] != '}')) return 0;
    }
    char * ops[] = {"+", "-", "*", "/", "%", "=", "!=", "?", "<", ">", "<=", ">=", "&&", "||", "!", "^^", "->", "++", "--", "&", "|", "^", ">>", "<<", ">>>", "<<<"};
    for (int i = 0; i < 26; i++) {
        if (streql(tokens[offset], ops[i])) return 1;
    }

    return 0;
}

int operator_min_args(char * op) {
    if (streql(op,"?")) return 3;
    return 2;
}

int operator_max_args(char * op) {
    if (streql(op, "!") || streql(op, "++") || streql(op, "--")) return 1;
    if (streql(op,"?")) return 3;
    if (streql(op, "==") ||streql(op, "!=")
    || streql(op, "<") || streql(op, ">")
    || streql(op, "<=") || streql(op, ">=")) return 2;
    //|| streql(op, "&&") || streql(op, "||")
    //|| streql(op, "^^") || streql(op, "->")) return 2;
    return -1;
}

int is_label(char * token) {
    for (int i = 0; i < labi; i++) {
        if (streql(token, labels[i])) return 1;
    }
    return 0;
}

int is_digit(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return 1;
        default:
            return 0;
    }
}

int is_number(char * token) {
    //int hasdot = 0;
    int strl = strlen(token);
    //strings
    if (token[0] == '\'' && strl == 3 && token[2] == '\'') {
        return 1;
    }
    //printf("%s is not string\n", token);
    if (token[0] == '\0') return 0;
    for (int i = 0; i < strlen(token); i++) {
        if (i == 0 && token[i] == '-' && strlen(token) > 1) continue;
        int dot = token[i] == '.';
        if (!is_digit(token[i])) {

            return 0;
        }
        //if (!isnumber(token[i]) && ((dot && hasdot) || !dot)) return 0;
        //if (dot) hasdot = 1;
    }
    return 1;
}


void print_expression(FILE * stream, char ** tokens, int count) {
    for (int i = 0; i < count; i++) {
        if (tokens[i][0] == '(' || (i != count - 1 && (tokens[i+1][0] == ')' || tokens[i + 1][0] == ';' || tokens[i + 1][0] == ':')))
            fprintf(stream, "%s", tokens[i]);
        else if (tokens[i][0] == ';' || tokens[i][0] == ':') {
            fprintf(stream, "%s\n", tokens[i]);
        }
        else fprintf(stream, "%s ", tokens[i]);
    }
}

void printn_expression(FILE * stream, char ** tokens, int count, int max_lines) {
    int lines = 0;
    for (int i = 0; i < count; i++) {
        if (tokens[i][0] == '(' || (i != count - 1 && (tokens[i+1][0] == ')' || tokens[i + 1][0] == ';' || tokens[i + 1][0] == ':')))
            fprintf(stream, "%s", tokens[i]);
        else if (tokens[i][0] == ';' || tokens[i][0] == ':') {
            lines++;
            if (lines >= max_lines) {
                fprintf(stream, "%s", tokens[i]);
                return;
            }
            fprintf(stream, "%s\n", tokens[i]);

        }
        else fprintf(stream, "%s ", tokens[i]);
    }
}


void error_expression(char ** tokens, int count, int * locations) {
    fprintf(stderr, "Error in %s at line %d:, Invalid expression: ", filename, locations[0]);
    printn_expression(stderr, tokens, count, 1);
    fprintf(stderr, "\n");
}

int valid_operation(char ** tokens, int count) {
    if ( (count == 1  || (count > 1 && tokens[1][0] == ',' || tokens[1][0] == ';')) && (is_number(tokens[0]) || is_variable(tokens[0])) ) return 1;
}

//returns 0 if false, returns len if true
int valid_expression(char ** tokens, int count){
    if ( (count == 1  || (count > 1 && tokens[1][0] == ',' || tokens[1][0] == ';')) && (is_number(tokens[0]) || is_variable(tokens[0])) ) return 1;
    int paren = 0;
    int op_ok = 0;
    int last_left = 0;
    int level = 0;
    int args[100];
    char * ops[100];
    for (int i = 0; i < count; i++) {
            if (i == 0) {
                if (tokens[i][0] == ';') {
                    fprintf(stderr, "Error: Expression is empty\n");
                    return 0;
                }
                if (tokens[i][0] != '(') {
                    if (tokens[i][0] == '\'') {
                        fprintf(stderr, "Error: Characters can only be length 1: ");
                        printn_expression(stderr, tokens, count, 1);
                        fprintf(stderr, "\n");
                        return 0;
                    }
                    fprintf(stderr, "Error: Expression must be enclosed in parenthesis: ");
                    printn_expression(stderr, tokens, count, 1);
                    fprintf(stderr, "\n");
                    return 0;
                }
                op_ok = 1;
                paren++;
            }
            else if (i == 1) {
               // printf("token: %s\n", *(tokens + i));
                if (is_operator(tokens + i, count - i)) {
                    args[level] = 0;
                    ops[level] = get_operator(tokens + i , count - i);
                    if (tokens[i][0] == '{'){
                        if (tokens[i+2][0] == '(') {
                            int sublen = valid_expression(tokens + i + 2, count - i - 2);
                            if (!sublen) {
                                return 0;
                            }
                            i += sublen - 1;
                        }
                        i += 3;
                    }
                    op_ok = 0;
                }
                else {
                    fprintf(stderr, "Error: Symbol %s is not a valid operator\n", tokens[i]);
                    return 0;
                }
            }
            else {
                if (tokens[i][0] == ')') {
                    //printf("operator %s used %d arguments\n", ops[level], args[level]);
                    int max = operator_max_args(ops[level]);
                    int min = max == 1 ? 1 : operator_min_args(ops[level]);
                    if (args[level] < min) {
                        fprintf(stderr, "Error: Operator %s requires more than %d argument\n", ops[level], min - 1);
                        return 0;
                    }
                    args[level] = 0;
                    level--;
                    args[level]++;
                    if (last_left) {
                        fprintf(stderr, "Error: () is not a valid expression\n");
                        return 0;
                    }
                    paren--;
                    op_ok = 0;
                    last_left = 0;
                    continue;
                }
                if (tokens[i][0] == '(') {
                    level++;

                    paren++;
                    op_ok = 1;
                    last_left = 1;
                    continue;
                }
                else last_left = 0;
                if (tokens[i][0] == ',' || tokens[i][0] == ';' || tokens[i][0] == ':' || tokens[i][0] == '}') {
                    if (paren == 0) return i;
                    fprintf(stderr, "Error: Expression has unmatched parenthesis\n");
                    return 0;
                }
                else if (is_operator(tokens + i, count - i)) {
                    if (op_ok) {
                        args[level] = 0;
                        ops[level] = get_operator(tokens + i, count - i);
                        if (tokens[i][0] == '{') {
                            if (tokens[i+2][0] == '(') {
                                int sublen = valid_expression(tokens + i + 2, count - i - 2);
                                if (!sublen) {
                                    return 0;
                                }
                                i += sublen - 1;
                            }
                            i += 3;
                        }
                        op_ok = 0;
                    }
                    else {
                        fprintf(stderr, "Error: Expression not in prefix notation, operator %s is in an invalid location\n", tokens[i]);
                        return 0;
                    }
                    continue;
                }
                else if (!is_number(tokens[i]) && !is_variable(tokens[i])) {
                    fprintf(stderr, "Error: Unrecognized symbol %s in expression\n", tokens[i]);
                    return 0;
                }
                else {
                    args[level]++;
                    int max = operator_max_args(ops[level]);
                    if (max != -1 && args[level] > max) {
                        fprintf(stderr, "Error: Operator %s takes at most %d arguments\n", ops[level], max);
                        return 0;
                    }
                    op_ok = 0;
                }
            }
    }
    if (paren == 0) return count;
    fprintf(stderr, "Error: Expression has unmatched parenthesis\n");
    return 0;
}


int keyword_declaration(char ** tokens) {
    return streql(tokens[0], "I64");
}

/*int keyword_comment(char ** tokens) {
    return tokens[0][0] == '#';
}*/



int valid_declaration(char ** tokens, int token_count, int * locations) {
    if (is_number(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: Invalid declaration, %s is not a valid variable name\n", filename, locations[1], tokens[1]);
        return 0;
    }
    else if (is_variable(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: Invalid declaration, variable name %s is already in use\n", filename, locations[1], tokens[1]);
        return 0;
    }
    else if (tokens[1][0] == '%') {
        fprintf(stderr, "Error in %s at line %d: Invalid declaration, variable name %s is not allowed\n", filename, locations[1], tokens[1]);
        return 0;
    }
    if (tokens[2][0] == ';') {
        return 1;
    }
    else if (token_count < 5) {
        fprintf(stderr, "Error in %s at line %d: Invalid declaration, statement is incomplete\n", filename,
                locations[1]);
        printn_expression(stderr, tokens, 3, 1);
        fprintf(stderr, "\n");
        return 0;
    }
    else if (tokens[2][0] != '=') {
        fprintf(stderr, "Error in %s at line %d: Invalid declaration, unexpected token: %s\n", filename,
                locations[2], tokens[2]);
        fprintf(stderr, "\n");
        return 0;
    }
    int len = valid_expression(tokens + 3, token_count - 3);
    if (!len) {
        fprintf(stderr, "Error in %s at line %d: Invalid declaration, expression is invalid: ", filename, locations[1] );
        printn_expression(stderr, tokens, 3, 1);
        fprintf(stderr, "\n");
        return 0;
    }
    else if (!streql(tokens[3 + len], ";")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete declaration, expected ; at end of statement: ", filename, locations[1]);
        printn_expression(stderr, tokens, 3, 1);
        fprintf(stderr, "\n");
        return 0;
    }
    return 1;
}

int keyword_modifier(char ** tokens) {
    return is_modifier(tokens[1]);
}

int valid_modifier(char ** tokens, int * locations, int token_count) {
    if (is_number(tokens[0])) {
        fprintf(stderr, "Error in %s at line %d: Instruction %s requires a variable on its left hand side, "
                        "found a number\n",
                filename, locations[0], tokens[1]);
        return 0;
    }
    else if (!is_variable(tokens[0])) {
        fprintf(stderr, "Error in %s at line %d: %s has not been declared\n", filename, locations[0], tokens[0]);
        return 0;
    }
    int len = valid_expression(tokens + 2,token_count - 2);
    if (!len) {
        error_expression(tokens + 2, token_count - 2, locations + 2);
        return 0;
    }
    else if (2 + len >= token_count || tokens[2+len][0] != ';') {
        fprintf(stderr, "Error in %s at line %d: Incomplete assignment instruction, expected ; at end of statement ", filename, locations[2]);
        printn_expression(stderr, tokens, token_count, 1);
        fprintf(stderr, "\n");
        return 0;
    }
    return 1;

}

int formatters_count(char * str) {
    int f = 0;
    int found = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '%') found = 1;
        else if (found) {
            switch (str[i]) {
                case ' ':
                    break;
                default:
                    f++;
            }
            found = 0;
        }
    }
    return f;
}

int keyword_str(char ** tokens) {
    return (tokens[0][0] == '\"');
}

int valid_str(char ** tokens, int * locations, int token_count) {
    if (tokens[0][strlen(tokens[0]) - 1] != '\"' ) {
        fprintf(stderr, "Error in %s at line %d: ", filename, locations[0]);
        printn_expression(stderr, tokens, token_count, 1);
        fprintf(stderr, " is an incomplete string\n");
        return 0;
    }
    int args = 0;
    int fcount = formatters_count(tokens[0]);
    if (fcount > 5) {
        fprintf(stderr, "Error in %s at line %d: Invalid print instruction, currently only 5 formatters are allowed\n", filename, locations[0]);
        return 0;
    }
    int strlen0 = strlen(tokens[0]);
    int newlen = (strlen0 + (fcount * 8));
    char * make_long = malloc(sizeof(char) * newlen);
    /*
    if (fcount > 2) {
        fprintf(stderr, "Error in %s at line %d: Invalid print instruction, currently only 2 formatters are allowed\n", filename, locations[0]);
        return 0;
    }
     */
    if (tokens[1][0] != ',' && fcount > 0) {
        fprintf(stderr, "Error in %s at line %d: Invalid print instruction, arguments must be seperated by commas\n", filename, locations[1]);
        return 0;
    }
    for (int i = 0, j = 0, form = 0; i < strlen0; i++, j++) {
        //printf("i%d: %c\n", i, tokens[0][i]);
        if (form && tokens[0][i] == 's') {
            fprintf(stderr, "Error in %s at line %d: Invalid print instruction, %s is not supported\n", filename, locations[1], "%s");
            return 0;
        }
        if ((form && tokens[0][i] != 'l')) {
            /*
            if (tokens[0][i] == 's') {
                make_long[j] = 'c';
                make_long[j+1] = '%';
                make_long[j+2] = 'c';
                make_long[j+3] = '%';
                make_long[j+4] = 'c';
                make_long[j+5] = '%';
                make_long[j+6] = 'c';
                make_long[j+7] = '%';
                make_long[j+8] = 'c';
                make_long[j+9] = '%';
                make_long[j+10] = 'c';
                make_long[j+11] = '%';
                make_long[j+12] = 'c';
                make_long[j+13] = '%';
                make_long[j+14] = 'c';
                j+=14;
                form = tokens[0][i+1] == '%';
                continue;
            }
             */
            make_long[j] = 'l';
            make_long[j+1] = 'l';
            //printf("%c", make_long[j]);
            form = 0;
            i--;
            continue;
        }
        else {
            make_long[j] = tokens[0][i];
            //printf("%c", make_long[j]);
        }
        if (tokens[0][i] == '%') form = 1;
        else form = 0;
        //if (i!= 0) printf("%s\n", make_long);
    }
    tokens[0] = make_long;
    if (fcount == 0) {
        return 1;
    }
    for (int i = 1, len = 0, comma = 1; i < token_count || i < fcount; i++, comma =! comma) {
        if (tokens[i][0] == ';') {
            break;
        }
        if (tokens[i][0] == '\"') {
            fprintf(stderr, "Error in %s at line %d: Invalid print instruction, string arguments are currently not allowed\n", filename, locations[1]);
            return 0;
        }
        if (comma) {
            if (tokens[i][0] == ',') continue;
            else {
                fprintf(stderr, "Error in %s at line %d: Invalid print instruction, arguments must be seperated by commas\n", filename, locations[1]);
                return 0;
            }
        }

        if (is_number(tokens[i]) || is_variable(tokens[i])) {
            args++;
            continue;
        }
        if (tokens[i][0] != '(') {
            fprintf(stderr, "Error in %s at line %d: Unrecognized symbol: %s",filename, locations[0], tokens[i]);
            return 0;
        }
        len = valid_expression(tokens + i, token_count - i);
        if (len) {
            args++;
            if (i == token_count - 1) {
                fprintf(stderr, "Error in %s at line %d: Incomplete print instruction, expected ; at end of statement: ", filename, locations[0]);
                printn_expression(stderr, tokens, token_count, 1);
                fprintf(stderr, "\n");
                return 0;
            }
            i += len - 1;
            //printn_expression(stderr, tokens + i, token_count - i, 1);
           // fprintf(stderr, "\n");
        }
        else {
            fprintf(stderr, "Error in %s at line %d: Invalid expression: ", filename, locations[1]);
            printn_expression(stderr, tokens + i, token_count - i, 1);
            fprintf(stderr, "\n");
            return 0;
        }
    }
    if (args < fcount) {
        fprintf(stderr, "Error in %s at line %d: Insufficient arguments for print instruction: ", filename, locations[0]);
        printn_expression(stderr, tokens, token_count, 1);
        fprintf(stderr, "\n");
        return 0;
    }
    else if (args > fcount) {
        fprintf(stderr, "Error in %s at line %d: Excess arguments for print instruction: ", filename, locations[0]);
        printn_expression(stderr, tokens, token_count, 1);
        fprintf(stderr, "\n");
        return 0;

    }
    return 1;

}


int keyword_putn(char ** tokens ) {
    return streql(tokens[0], "putn");
}

int valid_putn(char ** tokens, int * locations, int token_count) {
    int len = valid_expression(tokens + 1, token_count - 1);
    if (len == 0) {
        error_expression(tokens + 1, token_count - 1, locations + 1);
        return 0;
    }
    else if (!streql(tokens[len + 1], ";")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete goto if instruction, expected ; at end of statement\n", filename, locations[len]);
        return 0;
    }
    return 1;
}

int keyword_continue(char ** tokens) {
    return streql(tokens[0], "continue");
}

int valid_continue(char ** tokens, int * locations) {
    if (!streql(tokens[1], ";")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete continue instruction, expected ; at end of statement\n", filename,
                locations[0]);
        return 0;
    }
    return 1;
}

int keyword_label(char ** tokens) {
    return streql(tokens[0], "label");
}

int keyword_section(char ** tokens) {
    //return 0;
    return streql(tokens[0], "section");
}


int valid_label(char ** tokens, int * locations) {
    if (is_number(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: %s is not a valid label name\n", filename, locations[1], tokens[1]);
        return 0;
    }
    else if (is_label(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: label name %s is already in use\n", filename, locations[1], tokens[1]);
        return 0;
    }
    else if (!streql(tokens[2], ":")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete label, expected : at end of statement\n", filename, locations[1]);
        return 0;
    }
    return 1;
}

int keyword_goto(char ** tokens) {
    return streql(tokens[0], "goto");
}

int keyword_conditional(char ** tokens) {
    return streql(tokens[0], "if");
}

int keyword_if(char ** tokens) {
    return streql(tokens[2], "if");
}

int valid_goto(char ** tokens, int * locations, int line) {
    /*
    if (is_number(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: %s is not a valid label name\n", filename, locations[1], tokens[1]);
        return 0;
    }
    */
    if (is_number(tokens[1]) || tokens[1][0] == '+' && is_number(tokens[1] + 1)) {
        if (lls == 99) {
            fprintf(stderr, "Error in %s at line %d: maximum of 100 numeric gotos allowed\n", filename, locations[1], tokens[1]);
            return 0;
        }
        if (tokens[1][0] == '-') {
            if (add_numeric_label(line - atoi(tokens[1] + 1), line,  locations) == 0) return 0;
        }
        else if (tokens[1][0] == '+'){
            if (add_numeric_label(line + atoi(tokens[1] + 1), line,  locations) == 0) return 0;
        }
        else {
            if (add_numeric_label(atoi(tokens[1]), line,  locations) == 0) return 0;
        }
    }
    else if (!is_label(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: label %s does not exist\n", filename, locations[1], tokens[1]);
        return 0;
    }
    if (!streql(tokens[2], ";")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete goto instruction, expected ; at end of statement\n", filename, locations[1]);
        return 0;
    }
    return 1;
}

int valid_goto_if(char ** tokens, int * locations, int token_count, int line) {
    if (is_number(tokens[1]) || tokens[1][0] == '+' && is_number(tokens[1] + 1)) {
        if (lls == 99) {
            fprintf(stderr, "Error in %s at line %d: maximum of 100 numeric gotos allowed\n", filename, locations[1], tokens[1]);
            return 0;
        }
        if (tokens[1][0] == '-') {
            if (add_numeric_label(line - atoi(tokens[1] + 1), line,  locations) == 0) return 0;
        }
        else if (tokens[1][0] == '+'){
            if (add_numeric_label(line + atoi(tokens[1] + 1), line,  locations) == 0) return 0;
        }
        else {
            if (add_numeric_label(atoi(tokens[1]), line,  locations) == 0) return 0;
        }
    }
    else if (!is_label(tokens[1])) {
        fprintf(stderr, "Error in %s at line %d: label %s does not exist\n", filename, locations[1], tokens[1]);
        return 0;
    }
    int len = valid_expression(tokens + 3, token_count - 3);
    if (len == 0) {
        error_expression(tokens + 3, token_count - 3, locations + 3);
        return 0;
    }
    else if (!streql(tokens[len + 3], ";")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete goto if instruction, expected ; at end of statement\n", filename, locations[len + 2]);
        return 0;
    }
    return 1;
}

int valid_conditional(char ** tokens, int * locations, int token_count) {
    int len = 1;
    if (!is_number(tokens[1]) && !is_variable(tokens[1])) {
        int len = valid_expression(tokens + 1, token_count - 1);
        if (len == 0) {
            error_expression(tokens + 1, token_count - 1, locations + 1);
            return 0;
        }
    }
    else if (!streql(tokens[len + 1], ":")) {
        fprintf(stderr, "Error in %s at line %d: Incomplete conditional instruction, expected : at end of if statement\n", filename, locations[len + 2]);
        return 0;
    }
    return 1;
}

int valid_comment(char ** tokens, int * locations, int token_count) {
    do {
        tokens++;
        token_count--;
        if (tokens[0][0] == '#') {
            return 1;
        }
    } while (token_count > 0);
    fprintf(stderr, "Error in %s at line %d: Incomplete comment, comments must end and begin with #\n", filename, locations[0]);
    return 0;
}

void add_strings(char * name) {
    strings[stri] = name;
    stri++;
}

void add_variable(char * name) {
    variables[vari] = name;
    vari++;
}

void add_label(char * name) {
    labels[labi] = name;
    labi++;
}

int add_numeric_label(int gotoline, int currentline, int * locations) {
    if (gotoline == currentline) {
        fprintf(stderr, "Error in %s at line %d: goto cannot goto its own line\n", filename, locations[1]);
        return 0;
    }
    else if (gotoline > maxline) {
        fprintf(stderr, "Error in %s at line %d: goto cannot goto line that does not exist\n", filename, locations[1]);
        return 0;
    }
    for (int i = 0; i < lls; i++) {
        if (label_locations[i] == gotoline) return 1;
    }
    label_locations[lls] = gotoline;
    lls++;
    return 1;
}

/* Uncovered cases
 * goto undefined label
 *
 */

int validate_labels_and_vars(char ** tokens, int * locations, int token_count) {
    check_tokens:
    if (token_count == 0) {
        return 1;
    }
    else if (token_count > 2 && keyword_declaration(tokens)) {
        if (!valid_declaration(tokens, token_count, locations)) return 0;
        else add_variable(tokens[1]);
    }
    else if (token_count > 2 && keyword_label(tokens)) {
        if (!valid_label(tokens, locations)) return 0;
        else add_label(tokens[1]);
    }
    else if (token_count > 2 && keyword_section(tokens)) {
        if (!valid_label(tokens, locations)) return 0;
        else {
            add_label(tokens[1]);
            sections[seci] = continues;
            seci++;
        }
    }
    else if (token_count > 1 && keyword_continue(tokens)) {
        if (!valid_continue(tokens, locations)) return 0;
        else continues++;
    }
    for (int i = 0; i < token_count; i++) {
        if (tokens[i][0] == ':' || tokens[i][0] == ';') {
            token_count -= i + 1;
            tokens += i + 1;
            locations += i + 1;
            maxline++;
            goto check_tokens;
        }
    }
    return 1;
}

int validate_program(char ** tokens, int * locations, int token_count, char * f, int print_all) {
    int current_line = 1;
    char ** original_tokens = tokens;
    int original_count = token_count;
    filename = f;
    int commented = 0;
    if (!validate_labels_and_vars(tokens, locations, token_count)) return 0;
    check_tokens:
    if (token_count == 0) {
        if (print_all) print_expression(stdout, original_tokens,original_count );
        return 1;
    }
    /*
    else if (token_count > 1 && keyword_comment(tokens)) {
        if (valid_comment(tokens, locations, token_count)) {

        }
    }
     */
    else if (token_count > 3 && keyword_modifier(tokens)) {
        if (!valid_modifier(tokens, locations, token_count)){
            return 0;
        }
    }
    else if (token_count > 4 && keyword_goto(tokens) && keyword_if(tokens)) {
        if (!valid_goto_if(tokens,locations, token_count, current_line)){
            return 0;
        }
    }
    /*
    else if (token_count > 3 && keyword_conditional(tokens)) {
        if (!valid_conditional(tokens, locations, token_count)) {
            return 0;
        }
    }
     */
    else if (token_count > 2 && keyword_goto(tokens)) {
        if (!valid_goto(tokens,locations, current_line)) return 0;
    }
    else if (token_count > 2 && keyword_putn(tokens)) {
        if (!valid_putn(tokens, locations, token_count)) return 0;
    }
    else if (token_count > 1 && keyword_str(tokens)) {
        if (!valid_str(tokens, locations, token_count)) {
            return 0;
        }
        add_strings(tokens[0]);
    }
    else if (!(token_count > 4 && keyword_declaration(tokens)) &&  !(token_count > 2 && keyword_label(tokens)) &&
    !(token_count > 2 && keyword_section(tokens)) && !(token_count > 1 && keyword_continue(tokens))){
        fprintf(stderr, "Error in %s at line %d: Malformed instruction: ", filename, locations[0]);
        printn_expression(stderr, tokens, token_count, 1);
        fprintf(stderr,"\n");
        return 0;
    }
    for (int i = 1; i < token_count; i++) {
        if (tokens[i][0] == ':' || tokens[i][0] == ';' || tokens[i][0] == '#') {
            token_count -= i + 1;
            tokens += i + 1;
            locations += i + 1;
            current_line++;
            goto check_tokens;
        }
    }
    //printf("reminaing: %d\n", token_count);
    //print_expression(stdout, original_tokens,original_count );
    fprintf(stderr, "Error in %s at line %d: Unprocessed tokens", filename, locations[original_count - 1]);
    return 0;
}



//Compiler
void cmpl_unary_instruction(char * instruction, char * arg);

void cmpl_printf(char * arg1, char * arg2, char * arg3) {

}

void cmpl_comment() {
    //
}

void cmpl_putn(char * val) {
    fprintf(out, "\tmov %s, %s\n", "%rax", "%r11");
    fprintf(out, "\tmov %s, %s\n", "%rcx", "%r12");
    if (!streql("%rsi", val)) {
        fprintf(out, "\tmov %s, %s\n", val, "%rsi");
    }
    fprintf(out, "\tlea strcnst%d(%s), %s\n", stri - 1, "%rip", "%rdi");
    fprintf(out, "\tmov %s, %s\n", "$0", "%rax");
    fprintf(out, "\tcall _printf\n");
    fprintf(out, "\tmov %s, %s\n", "%r11", "%rax");
    fprintf(out, "\tmov %s, %s\n", "%r12", "%rcx");
}

int get_var_loc(char * varname) {
    for (int i = 0; i < 100; i++) {
        if (streql(variables[i], varname)) {
            return -8 * (i + 1);
        }
    }
    fprintf(stderr, "get_var_loc() failed\n");
    return 0;
}

int get_str_loc(char * strname) {
    for (int i = 0; i < 100; i++) {
        if (streql(strings[i], strname)) {
            return i;
        }
    }
    fprintf(stderr, "get_str_loc() failed\n");
    return -1;
}

void cmpl_string_constants(){
    for (int i = 0; i < stri; i++) {
        fprintf(out,"strcnst%d:\n", i);
        fprintf(out, "\t.asciz %s\n", strings[i]);
    }
    fprintf(out, "\n");
}

char * get_op(char * op) { return op; }

void int_to_string(char * intstring, char * out) {
    long long a = atoll(intstring);
    char arr[8];
    if (a < 0) {
        a -= 1;
        a = ~a;
    }
    //printf("is %s is %lld is string: \n", intstring, a);
    int last = 0;
    for (int i = 0; i < 8; ++i)
    {
        arr[i] = a & 0xFF;
        a >>= 8;
        if (a == 0) {
            last = i;
            break;
        }
        //printf("%d, %d, %c\n", a, arr[i], arr[i]);
        //printf("%d.", arr[i]);
    }
    int j = 0;
    for (int i = last; i >= 0; i--, j++) {
        out[j] = arr[i];
    }
    out[j] = '\0';
}

long long to_int(char * word) {
    long long out = 0;
    char array[8];
    for (int i = 1; i < strlen(word) - 1; i++) {
        array[i-1] = word[i];
        out <<= 8;
        out += array[i-1];
    }
   // printf("%s is %lld is ", word, out);
   // char out2[100];
   // sprintf(out2, "%lld", out);
   // char out3[8];
    //int_to_string(out2, out3);
    //printf("string: %s", out3);
    if (out > INT64_MAX) {
        out = ~out;
        out += 1;
    }
    long long temp = out;
    //printf("\n%d\n", out);
    return out;
}



void cmpl_unary_instruction(char * instruction, char * arg) {
    //0: number
    //1: variable
    //2: register
    int arg_type = -1;
    if (is_number(arg)) {
        if (arg[0] == '\'') {
            long long n = to_int(arg);
            sprintf(arg, "%d", n);
            //int_to_string(arg);
        }
        arg_type = 0;
    }
    else if (is_variable(arg)) arg_type = 1;
    else arg_type = 2;
    //assert(arg_type != -1);
    switch (arg_type) {
        case 0:
            fprintf(out, "\t%s $%s\n", instruction, arg);
            return;
        case 1:
            fprintf(out, "\t%s %d(%s)\n", instruction, get_var_loc(arg), "%rbp");
            return;
        case 2:
            fprintf(out, "\t%s %s\n", instruction, arg);
            return;
    }
    fprintf(stderr, "I don't know how this is being seen, but it shouldn't. cmpl_unary_instruction() failed.\n");
}

void cmpl_instruction(char * instruction, char * arg1, char * arg2) {
    //0: number
    //1: variable
    //2: register
    int arg1_type = -1;
    if (is_number(arg1)) {
        if (arg1[0] == '\'') {
            long long n = to_int(arg1);
            sprintf(arg1, "%d", n);
            //printf("\narg1: %lld\n",arg1);
        }
        arg1_type = 0;
    }
    else if (is_variable(arg1)) arg1_type = 1;
    else arg1_type = 2;
    //assert(arg1_type >= 0);
    int arg2_type = -1;
    if (is_number(arg2)) {
        if (arg2[0] == '\'') {
            long long n = to_int(arg1);
            sprintf(arg2, "%d", n);
        }
        arg2_type = 0;
    }
    else if (is_variable(arg2)) arg2_type = 1;
    else arg2_type = 2;
    //assert(arg2_type >= 0);
    if (arg1_type == 0 && arg2_type == 0) {
        fprintf(out, "\t%s $%s, $%s\n", instruction, arg1, arg2);
    }
    else if (arg1_type == 0 && arg2_type == 1) {
        fprintf(out, "\t%s $%s, %d(%s)\n", instruction, arg1, get_var_loc(arg2), "%rbp");
    }
    else if (arg1_type == 0 && arg2_type == 2) {
        fprintf(out, "\t%s $%s, %s\n", instruction, arg1, arg2);
    }
    else if (arg1_type == 1 && arg2_type == 0) {
        fprintf(out, "\t%s %d(%s), $%s\n", instruction, get_var_loc(arg1), "%rbp" , arg2);
    }
    else if (arg1_type == 1 && arg2_type == 1) {
        fprintf(out, "\t%s %d(%s), %d(%s)\n", instruction, get_var_loc(arg1), "%rbp", get_var_loc(arg2), "%rbp");
    }
    else if (arg1_type == 1 && arg2_type == 2) {
        fprintf(out, "\t%s %d(%s), %s\n", instruction, get_var_loc(arg1), "%rbp", arg2);
    }
    else if (arg1_type == 2 && arg2_type == 0) {
        fprintf(out, "\t%s %s, $%s\n", instruction, arg1, arg2);
    }
    else if (arg1_type == 2 && arg2_type == 1) {
        fprintf(out, "\t%s %s, %d(%s)\n", instruction, arg1, get_var_loc(arg2), "%rbp");
    }
    else if (arg1_type == 2 && arg2_type == 2) {
        fprintf(out, "\t%s %s, %s\n", instruction, arg1, arg2);
    }
    //fprintf(stderr, "I don't know how this is being seen, but it shouldn't. cmpl_instruction() failed.\n");

}

;

//Collected value is in %rax, should output to %rax. Use %rbx, %r9 as scratch.
//recursive operation
void cmpl_operation(char * op_token, char * value) {
    static int equivs = 0;
    if (streql(op_token, "+")) {
        cmpl_instruction("addq", value, "%rax");
    }
    else if (streql(op_token, "-")) {
        cmpl_instruction("subq", value, "%rax");
    }
    else if (streql(op_token, "--")) {
        cmpl_instruction("subq", "1", "%rax");
    }
    else if (streql(op_token, "++")) {
        cmpl_instruction("addq", "1", "%rax");
    }
    else if (streql(op_token, "*")) {
        cmpl_instruction("imulq", value, "%rax");
    }
    else if (streql(op_token, ">>")) {
        cmpl_instruction("shrq", value, "%rax");
    }
    else if (streql(op_token, "<<")) {
        cmpl_instruction("shlq", value, "%rax");
    }
    else if (streql(op_token, ">>>")) {
        cmpl_instruction("sarq", value, "%rax");
    }
    else if (streql(op_token, "<<<")) {
        cmpl_instruction("salq", value, "%rax");
    }
    else if (streql(op_token, "&")) {
        cmpl_instruction("andq", value, "%rax");
    }
    else if (streql(op_token, "|")) {
        cmpl_instruction("orq", value, "%rax");
    }
    else if (streql(op_token, "^")) {
        cmpl_instruction("xorq", value, "%rax");
    }
    else if (streql(op_token, "/")) {
        if (is_number(value)) {
            cmpl_instruction("mov", value, "%rbx");
            fprintf(out, "\tcdq\n");
            cmpl_unary_instruction("idivq", "%rbx");
        }
        else {
            fprintf(out, "\tcdq\n");
            cmpl_unary_instruction("idivq", value);
        }

    }
    else if (streql(op_token, "%")) {
        if (is_number(value)) {
            cmpl_instruction("mov", value, "%rbx");
            fprintf(out, "\tcdq\n");
            cmpl_unary_instruction("idivq", "%rbx");
            cmpl_instruction("mov", "%rdx", "%rax");
        }
        else {
            fprintf(out, "\tcdq\n");
            cmpl_unary_instruction("idivq", value);
            cmpl_instruction("mov", "%rdx", "%rax");
        }
    }
    else if (streql(op_token, "&&")) {
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_unary_instruction("not", "%rax");
        cmpl_instruction("mov", value, "%rbx");
        cmpl_instruction("cmp", "0", "%rbx");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rbx");
        cmpl_unary_instruction("not", "%rbx");
        cmpl_instruction("andq", "%rbx", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, "||")) {
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_unary_instruction("not", "%rax");
        cmpl_instruction("mov", value, "%rbx");
        cmpl_instruction("cmp", "0", "%rbx");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rbx");
        cmpl_unary_instruction("not", "%rbx");
        cmpl_instruction("orq", "%rbx", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, "!")) {
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, "^^")) {
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_unary_instruction("not", "%rax");
        cmpl_instruction("mov", value, "%rbx");
        cmpl_instruction("cmp", "0", "%rbx");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rbx");
        cmpl_unary_instruction("not", "%rbx");
        cmpl_instruction("xorq", "%rbx", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, "->")) {
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("mov", value, "%rbx");
        cmpl_instruction("cmp", "0", "%rbx");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rbx");
        cmpl_unary_instruction("not", "%rbx");
        cmpl_instruction("orq", "%rbx", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, ">=")) {
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_unary_instruction("not", "%rax");
        cmpl_instruction("andq", "128", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "128", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, "<=")) {
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "192", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_unary_instruction("not", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, ">")) {
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "192", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");

    }
    else if (streql(op_token, "<")) {
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "128", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "128", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
    }
    else if (streql(op_token, "=")) {
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
        //cmpl_putn("%rax");
        /*
        cmpl_instruction("mov", "%rcx", "%rbx");
        cmpl_instruction("subq", value, "%rbx");
        cmpl_instruction("mov", "%rbx", "%r9");
        cmpl_instruction("addq", "1", "%r9");
        cmpl_instruction("xorq", "%rbx", "%r9");
        cmpl_instruction("mov", "%r9", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%r9");
         */

    }
    else if (streql(op_token, "!=")) {
       // cmpl_putn("%rcx");
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rax");
        cmpl_instruction("andq", "64", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%rbx");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%rbx");
        cmpl_instruction("xor", "1","%rax");

       /*
        cmpl_instruction("cmp", value, "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%r9");
        cmpl_instruction("andq", "64", "%r9");
        cmpl_instruction("mov", "%r9", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        cmpl_instruction("mov", "64", "%r9");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%r9");
        cmpl_unary_instruction("not", "%rax");
        cmpl_instruction("and","1", "%rax");
        */
    }
    else if (streql(op_token, "?")) {
        static int first = 1;
        if (first) {
            cmpl_instruction("cmp", "0", "%rax");
            fprintf(out, "\tpushfq\n");
            cmpl_unary_instruction("pop", "%rcx");
            cmpl_unary_instruction("not",  "%rcx");
            cmpl_instruction("andq", "64", "%rcx"); //rcx is 64 if rax is NOT zero
            cmpl_instruction("mov", "%rcx", "%rax");
            cmpl_instruction("mov", "0", "%rdx");
            fprintf(out, "\tcdq\n");
            cmpl_instruction("mov", "-64", "%r8");
            cmpl_unary_instruction("idivq", "%r8"); //rax is -1 if rax is NOT zero, 0 if zero
            cmpl_instruction("mov", "%rax", "%r14");
            cmpl_instruction("andq", value, "%rax");
        }
        else {
            static int j = 0;
            //if r14 is zero, rax <- value
            //if r14 is -1, rax <- rax
            cmpl_instruction("cmp", "0", "%r14");
            fprintf(out, "\tjne end%d\n", j);
            //value
            cmpl_instruction("mov", value, "%rax");
            fprintf(out, "\tend%d:\n", j);
            j++;
        }
        first = (first + 1) % 2;

        /*
        //first rax is in r10
        cmpl_instruction("mov", "%rax", "%r10");
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rcx");
        cmpl_unary_instruction("not",  "%rcx");
        cmpl_instruction("andq", "64", "%rcx");
        cmpl_instruction("mov", "%rcx", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        fprintf(out, "\tcdq\n");
        cmpl_instruction("mov", "-64", "%r8");
        cmpl_unary_instruction("idivq", "%r8");
        //cmpl_putn("%r15");
        //r15 is first pass?, should set rax to val if rax == r15

        cmpl_instruction("cmp", "%rax", "%r15");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rcx");
        cmpl_instruction("andq", "64", "%rcx");
        cmpl_instruction("mov", "%rcx", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        fprintf(out, "\tcdq\n");
        cmpl_instruction("mov", "-64", "%r8");
        cmpl_unary_instruction("idivq", "%r8");
        cmpl_instruction("mov", "%rax", "%rcx");
        //rcx is -1 if rax == r15
        //cmpl_putn("%rcx");
        cmpl_instruction("mov", "%r10", "%rax");
        cmpl_instruction("mov", "0", "%r15");
        //r15 is true if rax is true and it is the first pass


        /*
        cmpl_instruction("cmp", "0", "%r12"); //assumed rax is backed up in rbx, work with rax in r13
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rcx");
        cmpl_unary_instruction("not",  "%rcx");
        cmpl_instruction("andq", "64", "%rcx");
        cmpl_instruction("mov", "%rcx", "%rax");
        cmpl_instruction("mov", "0", "%rdx");
        fprintf(out, "\tcdq\n");
        cmpl_instruction("mov", "-64", "%r8");
        cmpl_unary_instruction("idivq", "%r8");
        cmpl_instruction("andq", "%rax", "%r15");
        cmpl_instruction("orq", "%r15", "%r14"); //r14 is conditional and future is assigned
       // cmpl_instruction("mov", "0", "%r15");
        cmpl_instruction("cmp", "0", "%r15");
        static int i = 0;
        fprintf(out, "\tje rax%d\n", i);
        fprintf(out, "\tjne val%d\n", i);
        cmpl_instruction("mov", "-2", "%rax");
        fprintf(out, "\tjmp end%d\n", i);
        fprintf(out, "\trax%d:\n", i);
        cmpl_instruction("mov", "%r12", "%rax"); //%rbx
        fprintf(out, "\tjmp end%d\n", i);
        fprintf(out, "\tval%d:\n", i);
        cmpl_instruction("mov", value, "%rax"); //value
        fprintf(out, "\tend%d:\n", i);

        i++;
        /*
        cmpl_instruction("cmp", "0", "%rax");
        fprintf(out, "\tpushfq\n");
        cmpl_unary_instruction("pop", "%rbx");
        cmpl_instruction("andq", "64", "%rbx");
        cmpl_instruction("mov", "%rax", "%r15");
        cmpl_instruction("mov", "-64", "%r14");
        cmpl_instruction("mov", "%rbx", "%rax");
        fprintf(out, "\tcdq\n");
        cmpl_unary_instruction("idivq", "%r14");
        cmpl_instruction("mov", "%rax", "%rbx");
        cmpl_instruction("mov", "%r15", "%rax");
         */
    }

}

//Calculate into %rax, use %rbx as scratch
void cmpl_expression(char ** tokens, int token_count, char * register_result) {
    if (streql(tokens[1],";") || streql(tokens[1],",") || streql(tokens[1],":")){
        cmpl_instruction("mov", tokens[0], register_result);
        return;
    }
    char * op_token_stack[100] = {"empty"};
    int argn_stack[100] = {0};
    int level = 0;
    int should_push = 0;
    static int repeaters = 0;
    int in_repeater = 0;
    char * repeated_tokens[1000];
    int rti = 0;
    for (int i = 1; i < token_count; i++) {
        if (tokens[i][0] == '(') {
            level ++;
        }
        else if (tokens[i][0] == ')') {
            if (in_repeater) {
                cmpl_instruction("cmp", "1", "%r15");
                fprintf(out, "\tjg repeater%d\n", repeaters);
                fprintf(out, "\tje post_repeater%d\n", repeaters);
                cmpl_instruction("mov", "%r13", "%rax");
                fprintf(out, "\tjmp post_repeater%d\n", repeaters);
                fprintf(out, "\trepeater%d:\n", repeaters);
                for (int j = 0; j < rti; j++) {
                    cmpl_operation(op_token_stack[level], repeated_tokens[j]);
                }
                rti = 0;
                cmpl_instruction("sub", "1", "%r15");
                cmpl_instruction("cmp", "1", "%r15");
                fprintf(out, "\tjg repeater%d\n", repeaters);
                fprintf(out, "\tpost_repeater%d:\n", repeaters);
                repeaters++;
                in_repeater = 0;
            }
            if (argn_stack[level] == 1) {
                cmpl_operation(op_token_stack[level], "%rax");
            }
            argn_stack[level] = 0;
            level --;
            if (level == -1) {
                cmpl_instruction("mov", "%rax", register_result);
                return;
            }
            if (argn_stack[level] > 0) {
                cmpl_instruction("mov", "%rax","%rbx");
                cmpl_unary_instruction("pop", "%rax");
                cmpl_operation(op_token_stack[level], "%rbx");
            }
            argn_stack[level]++;
        }
        else if (is_number(tokens[i]) || is_variable(tokens[i])) {
            if (argn_stack[level] == 0) {
                if (should_push) {
                    cmpl_unary_instruction("push", "%rax");
                }
                cmpl_instruction("mov", tokens[i], "%rax");
                if (in_repeater) cmpl_instruction("mov", "%rax", "%r13");
            }
            else {
                if (in_repeater) {
                    repeated_tokens[rti] = tokens[i];
                    rti++;
                }
                cmpl_operation(op_token_stack[level], tokens[i]);
            }
            should_push = 1;
            argn_stack[level]++;
        }
        else if (is_operator(tokens + i, token_count - i)) {
            char * op = get_operator(tokens + i, token_count - i);
            in_repeater = tokens[i][0] == '{';
            int sublen = 1;
            if (in_repeater){
                if (tokens[i+2][0] == '('){
                    cmpl_unary_instruction("push", "%rax");
                    cmpl_expression(tokens + i + 2, token_count - i - 2, "%r15");
                    sublen = valid_expression(tokens + i + 2, token_count - i - 2);
                    cmpl_unary_instruction("pop", "%rax");
                }
                else cmpl_instruction("mov", tokens[i+2], "%r15");
                i += 2 + sublen;
            }
            if (streql(op, "?")) {
                cmpl_instruction("mov", "0", "%r14"); //?: has assigned?
            }
            op_token_stack[level] = op;
        }
        else {
            fprintf(stderr, "Invalid %s token in cmpl_expression(). This should not be seen. %s\n", tokens[i], tokens[0]);
            printn_expression(stderr, tokens, token_count, 1);
            fprintf(stderr, "\n");

            return;
        }
    }
}


/*
void cmpl_expression(char ** tokens, int token_count, char * register_result, FILE * out) {
    if (streql(tokens[1],";") || streql(tokens[1],",")){
        if (is_number(tokens[0])) {
            fprintf(out, "\tmov $%s, %s\n", tokens[0], register_result);
        }
        else {
            fprintf(out, "\tmov %d(%s), %s\n", get_var_loc(tokens[0]), "%rbp", register_result);
        }
        return;
    }
    char * op_stack[100];
    int argn_stack[100] = {0};
    int op_num = 0;
    argn_stack[0] = 1;
    for (int i = 1; i < token_count; i++, (argn_stack[op_num])++) {
        if (is_operator(tokens[i])) {
            switch (tokens[i][0]) {
                case '+':
                    op_stack[op_num] = "addq";
                    break;
                case '-':
                    op_stack[op_num] = "subq";
                    break;
                case '*':
                    op_stack[op_num] = "imulq";
                    break;
                case '/':
                    op_stack[op_num] = "idivq";
                    break;
                case '%':
                    op_stack[op_num] = "MOD";
                    break;
            }
        }
        else if (tokens[i][0] == '(') {
            op_num++;
            argn_stack[op_num] = 0;
            continue;
        }
        else if (tokens[i][0] == ')') {
            if (op_num == 0) {
                fprintf(out, "\tmov %s, %s\n", "%rax", register_result);
                return;
            }
            char ar = op_num == 0 || argn_stack[op_num - 1] != 2;
            op_num--;

            if (ar) fprintf(out, "\tpop %s\n", "%rcx");
            argn_stack[op_num]++;
            //fprintf(out, "\t%s %s, %s\n", op_stack[op_num], "%rcx", "%rax");
            char mod = streql(op_stack[op_num],"MOD");
            if (streql(op_stack[op_num],"idivq") || mod) {
                if (ar) {
                    fprintf(out, "\tpush %s\n", "%rax");
                    fprintf(out, "\t%s %s, %s\n", "mov", "%rcx", "%rax");
                    fprintf(out, "\tpop %s\n", "%rcx");
                    fprintf(out, "\tcdq\n");
                    fprintf(out, "\t%s %s\n", "idivq", "%rcx");
                    if (mod) fprintf(out, "\tmov %s, %s\n", "%rdx", "%rax");
                }
            }
            else if (ar) {
                fprintf(out, "\t%s %s, %s\n", op_stack[op_num], "%rax", "%rcx");
                fprintf(out, "\tmov %s, %s\n", "%rcx", "%rax");
            }
            if (streql(op_stack[op_num],"first")) {
                op_num--;
            }
        }
        else {
            if (is_number(tokens[i])) {
                if (argn_stack[op_num] == 2) {
                    if (op_num != 0 && argn_stack[op_num-1] != 2) fprintf(out, "\tpush %s\n", "%rax");
                    fprintf(out, "\tmov $%s, %s\n", tokens[i], "%rax");
                    continue;
                }
                char mod = streql(op_stack[op_num],"MOD");
                if (streql(op_stack[op_num],"idivq") || mod) {
                    fprintf(out, "\tpush %s\n", "%r8");
                    fprintf(out, "\t%s $%s, %s\n", "mov", tokens[i], "%r8");
                    fprintf(out, "\tcdq\n");
                    fprintf(out, "\t%s %s\n", "idivq", "%r8");
                    fprintf(out, "\tpop %s\n", "%r8");
                    if (mod) fprintf(out, "\tmov %s, %s\n", "%rdx", "%rax");
                }
                else fprintf(out, "\t%s $%s, %s\n", op_stack[op_num], tokens[i], "%rax");
            }
            else {
                if (argn_stack[op_num] == 2 && argn_stack[op_num-1] != 2) {
                    if (op_num != 0 && argn_stack[op_num-1] != 2)  fprintf(out, "\tpush %s\n", "%rax");
                    fprintf(out, "\tmov %d(%s), %s\n", get_var_loc(tokens[i]), "%rbp", "%rax");
                    continue;
                }
                char mod = streql(op_stack[op_num],"MOD");
                if (streql(op_stack[op_num],"idivq")) {
                    fprintf(out, "\tcdq\n");
                    fprintf(out, "\t%s %d(%s)\n", op_stack[op_num], get_var_loc(tokens[i]), "%rbp");
                    if (mod) fprintf(out, "\tmov %s, %s\n", "%rdx", "%rax");
                }
                else {
                    printf("var_loc %d\n", get_var_loc(tokens[i]));
                    fprintf(out, "\tmov %d(%s), %s\n", get_var_loc(tokens[i]), "%rbp", "%rcx");
                    fprintf(out, "\t%s %s, %s\n", op_stack[op_num], "%rcx", "%rax");
                }
            }
        }
    }

}

void cmpl_expression(char ** tokens, int token_count, char * register_result, FILE * out) {
    if (streql(tokens[1],";") || streql(tokens[1],",")){
        if (is_number(tokens[0])) {
            fprintf(out, "\tmov $%s, %s\n", tokens[0], register_result);
        }
        else {
            fprintf(out, "\tmov %d(%s), %s\n", get_var_loc(tokens[0]), "%rbp", register_result);
        }
        return;
    }

    fprintf(out, "\tmov $0, %s\n", "%rax");
    char * op_stack[100];
    char op_num = 0;
    char first_new = 1;
    for (int i = 1; i < token_count; i++) {
        char * op;
        if (first_new){
            op = "addq";
        }
        else op = op_stack[op_num];
        if (tokens[i][0] == ',' || tokens[i][0] == ';') {
            return;
        }
        else if (is_operator(tokens[i])) {
            switch (tokens[i][0]) {
                case '+':
                    op_stack[op_num] = "addq";
                    break;
                case '-':
                    op_stack[op_num] = "subq";
                    break;
                case '*':
                    op_stack[op_num] = "imulq";
                    break;
                case '/':
                    op_stack[op_num] = "idivq";
                    break;
                case '%':
                    op_stack[op_num] = "MOD";
                    break;
            }
            continue;
        }
        else if (tokens[i][0] == '(') {
            op_num++;
            fprintf(out, "\tpush %s\n", "%rax");
            fprintf(out, "\tmov $0, %s\n", "%rax");
            first_new = 1;
        }
        else if (tokens[i][0] == ')') {
            if (op_num == 0) {
                fprintf(out, "\tmov %s, %s\n", "%rax", register_result);
                return;
            }
            op_num--;
            fprintf(out, "\tpop %s\n", "%rcx");
            //fprintf(out, "\t%s %s, %s\n", op_stack[op_num], "%rcx", "%rax");
            char mod = streql(get_op(op_stack[op_num]),"MOD");
            if (streql(get_op(op_stack[op_num]),"idivq") || mod) {
                fprintf(out, "\tpush %s\n", "%rax");
                fprintf(out, "\t%s %s, %s\n", "mov", "%rcx", "%rax");
                fprintf(out, "\tpop %s\n", "%rcx");
                fprintf(out, "\tcdq\n");
                fprintf(out, "\t%s %s\n", "idivq", "%rcx");
                if (mod) fprintf(out, "\tmov %s, %s\n", "%rdx", "%rax");
            }
            else fprintf(out, "\t%s %s, %s\n", get_op(op_stack[op_num]), "%rcx", "%rax");
            if (streql(op_stack[op_num],"first")) {
                op_num--;
            }
        }
        else {
            if (is_number(tokens[i])) {
                char mod = streql(op,"MOD");
                if (streql(op,"idivq") || mod) {
                    fprintf(out, "\tpush %s\n", "%r8");
                    fprintf(out, "\t%s $%s, %s\n", "mov", tokens[i], "%r8");
                    fprintf(out, "\tcdq\n");
                    fprintf(out, "\t%s %s\n", "idivq", "%r8");
                    fprintf(out, "\tpop %s\n", "%r8");
                    if (mod) fprintf(out, "\tmov %s, %s\n", "%rdx", "%rax");
                }
                else fprintf(out, "\t%s $%s, %s\n", get_op(op), tokens[i], "%rax");
            }
            else {
                char mod = streql(op,"MOD");
                if (streql(op,"idivq")) {
                    fprintf(out, "\tcdq\n");
                    fprintf(out, "\t%s %d(%s)\n", op_stack[op_num], get_var_loc(tokens[i]), "%rbp");
                    if (mod) fprintf(out, "\tmov %s, %s\n", "%rdx", "%rax");
                }
                else {
                    printf("var_loc %d\n", get_var_loc(tokens[i]));
                    fprintf(out, "\tmov %d(%s), %s\n", get_var_loc(tokens[i]), "%rbp", "%rcx");
                    fprintf(out, "\t%s %s, %s\n", get_op(op), "%rcx", "%rax");
                }
            }
            if (first_new) first_new = 0;
        }
    }
    fprintf(out, "\tpop %s\n", register_result);

}
 */

void cmpl_modifier(char * modifier, char * src, char * dst) {
    switch(modifier[0]) {
        case '=' :
            cmpl_instruction("mov", src, dst);

            break;
        case '+':
            cmpl_instruction("addq", src, dst);
            break;
        case '-':
            cmpl_instruction("subq", src, dst);
            break;
        case '/':
            cmpl_instruction("mov", dst, "%rax");
            cmpl_instruction("mov", "0", "%rdx");
            fprintf(out, "\tcdq\n");
            cmpl_unary_instruction("idivq", src);
            cmpl_instruction("mov", "%rax", dst);
            break;
        case '*':
            cmpl_instruction("mov", dst, "%rax");
            cmpl_instruction("imulq", src, "%rax");
            cmpl_instruction("mov", "%rax", dst);
            break;
    }
}

int cmpl_goto(char * label) {
    if (is_label(label)) {
        fprintf(out,"\tjmp %s\n", label);
        return 0;
    }
    else {
        fprintf(out, "\tjmp label%d\n", label_locations[current_numeric_label]);
        current_numeric_label++;
    }
}

int cmpl_goto_if(char * label) {
    if (is_label(label)) {
        fprintf(out,"\tjne %s\n", label);
        return 0;
    }
    else {
        fprintf(out, "\tjne label%d\n", label_locations[current_numeric_label]);
        current_numeric_label++;
    }
}

int compile_program(char ** tokens, int token_count, char * outname) {
    out = fopen(outname,"w");
    add_strings("\"%d\\n\"");
    fprintf(out,".data\n\n");
    fprintf(out,".align 16\n\n");
    cmpl_string_constants();
    fprintf(out,".text\n\n");
    fprintf(out,".global _main\n\n");
    fprintf(out,"_main:\n");
    fprintf(out, "\tpush %s\n", "%rbp");
    fprintf(out, "\tmov %s, %s\n", "%rsp", "%rbp");
    fprintf(out, "\tsub $%d, %s\n", 16*vari, "%rsp");
    int section = 0;
    int current_con = 0;
    int current_sec = 0;
    int conditionals = 0;
    int in_cond = 0;
    int current_line = 1;
    compile_tokens:
    if (in_cond == 1) in_cond = 2;
    if (token_count == 0) {
        if (section == 1) fprintf(out, "\tcontinue%d:\n", current_con);
        fprintf(out,"\tleave\n");
        fprintf(out, "\tret\n");
        fclose(out);
        return 1;
    }
    if (lls > 0) {
        for (int i = 0; i < lls; i++) {
            if (current_line == label_locations[i]) {
                fprintf(out, "\tlabel%d:\n", current_line);
            }
        }
    }
    if (keyword_label(tokens)) {
        fprintf(out, "\t%s:\n", tokens[1]);
    }
    else if (keyword_declaration(tokens) && tokens[2][0] != ';') {
        cmpl_expression(tokens + 3, token_count - 3, "%r8");
        char dst[15];
        snprintf(dst, 15, "%d(%s)", get_var_loc(tokens[1]), "%rbp");
        cmpl_instruction("mov", "%r8", dst);
    }
    /*
    else if (keyword_conditional(tokens)) {
        cmpl_expression(tokens + 1, token_count - 1, "%r8");
        cmpl_instruction("cmpq", "0", "%r8");
        fprintf(out,"\tje cond%d\n", conditionals);
        in_cond = 1;
    }
     */
    else if (keyword_section(tokens)) {
        section = 1;
        fprintf(out,"\tjmp continue%d\n", sections[current_sec]);
        fprintf(out, "\t%s:\n", tokens[1]);
        current_sec++;
    }
    else if (keyword_modifier(tokens)) {
        cmpl_expression(tokens + 2, token_count - 2, "%r8");
        char dst[15];
        snprintf(dst, 15, "%d(%s)", get_var_loc(tokens[0]), "%rbp");
        cmpl_modifier(tokens[1], "%r8", dst);
    }
    else if (keyword_goto(tokens) && keyword_if(tokens)) {
        cmpl_expression(tokens + 3, token_count - 3, "%r8");
        cmpl_instruction("cmpq", "0", "%r8");
        cmpl_goto_if(tokens[1]);
    }
    else if (keyword_goto(tokens)) {
        cmpl_goto(tokens[1]);
    }
    else if (keyword_continue(tokens)) {
        fprintf(out, "\tcontinue%d:\n", current_con);
        current_con++;
    }
    else if (keyword_putn(tokens)) {
        //printn_expression(stderr, tokens + 1, token_count - 3, 1);
        cmpl_expression(tokens + 1, token_count - 1, "%rsi");
        cmpl_putn("%rsi");
    }
    else if (keyword_str(tokens)) {
        fprintf(out, "\tlea strcnst%d(%s), %s\n", get_str_loc(tokens[0]), "%rip", "%rdi");
        int formatters = formatters_count(tokens[0]);
       // printf("formatters count: %d", formatters);
        int len = 0, sum = 0;
        for (int i = 0; i < formatters; i++) {
            len = valid_expression(tokens + 2 + sum, token_count - (2 + sum)) + 1;
            cmpl_expression(tokens + 2 + sum, token_count - (2 + sum), get_arg_register(i + 1));
            sum += len;
        }

        /*
        loop:

        if (formatters > 0) {
            if (len == 0) len = valid_expression(tokens + 2 + sum, token_count - (2 + sum)) + 1;
            cmpl_expression(tokens + 2 + sum, token_count - (2 + sum), "%rsi");
            sum += len;
            formatters--;
            goto loop;
        }
        /*
        if (formatters > 1) {
            cmpl_expression(tokens + 2 + len, token_count - (2 + len), "%rdx");
        }
         */
        cmpl_instruction("mov", "0", "%rax");
        fprintf(out, "\tcall _printf\n");

    }
    if (in_cond == 2) {
        fprintf(out, "\tcond%d:\n", conditionals);
        conditionals++;
        in_cond = 0;
    }
    for (int i = 0; i < token_count; i++) {
        if (tokens[i][0] == ':' || tokens[i][0] == ';') {
            token_count -= i + 1;
            tokens += i + 1;
            current_line ++;
            goto compile_tokens;
        }
    }
    fprintf(stderr, "Token processing error, this should not be seen\n");
    return 0;
}






/*
for (int i = 0; i < line_count; i++) {
    if (hanging_expression(lines[i])){
        continue;
    }
    else if (lines[i].token_count > 1) {
        if (keyword_declaration(lines[i])) {
            if (!valid_declaration(lines[i])) return 0;
        }
    }

}
 */
/*
int remaining_tokens = token_count;
char ** check_tokens = tokens;
 */