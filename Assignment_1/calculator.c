#include <stdio.h>
#include <ctype.h>
#include <string.h>

int numDigits(const char *str, int *p){
    int num = 0;
    while (isdigit(str[*p])){
        num = num * 10 + (str[*p] - '0');
        (*p)++;
    }
    (*p)--;
    return num;
}

void evaluate(const char *str){
    int p = 1;
    int num[128], c = 0;
    char op[128], o = 0;
    int start_pos = 0;
    if (str[0] == '-'){
        num[c++] = -numDigits(str, &p);
        start_pos = p + 1;
    }
    if (!isdigit(str[strlen(str) - 2])){
        printf("Foo ERROR: Invalid Expression.\n");
        return;
    }
    for (int i = start_pos; i < strlen(str) - 1; i++){
        if (str[i] == ' '){
            continue;
        }
        if (isdigit(str[i])){
            num[c++] = numDigits(str, &i);
            // printf("d-%d: %d\n", c - 1, num[c - 1]); //! DEBUG: Digits parsed
        }
        else if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/'){
            if (i == 0 && str[i] == '-'){
                continue;
            }
            if (str[i + 1] == '+' || str[i + 1] == '-' || str[i + 1] == '*' || str[i + 1] == '/'){
                printf("Test ERROR: Invalid Expression. %d\n", i);
                return;
            }
            op[o++] = str[i];
            // printf("o-%d: %c\n", o - 1, op[o - 1]); //! DEBUG: Operators parsed
        }
        else{
            printf("Global ERROR: Invalid Expression. %d\n", i);
            return;
        }
        // printf("%c : num[%d]: %d\n", str[i], c - 1, num[c - 1]); //! Test DEBUG 
    }
    for (int i = 0; i < o; i++){
        if (op[i] == '*' || op[i] == '/'){
            if (op[i] == '*'){
                num[i] = num[i] * num[i + 1];
            }
            else{
                if (num[i + 1] == 0){
                    printf("ERROR: Division by zero.\n");
                    return;
                }
                num[i] = num[i] / num[i + 1];
            }
            for (int j = i + 1; j < c - 1; j++){
                num[j] = num[j + 1];
            }
            for (int j = i; j < o - 1; j++){
                op[j] = op[j + 1];
            }
            c--;
            o--;
            i--;
        }
    }
    for (int i = 0; i < o; i++){
        if (op[i] == '+'){
            num[i] = num[i] + num[i + 1];
        }
        else if (op[i] == '-'){
            num[i] = num[i] - num[i + 1];
        }
        for (int j = i + 1; j < c - 1; j++){
            num[j] = num[j + 1];
        }
        for (int j = i; j < o - 1; j++){
            op[j] = op[j + 1];
        }
        c--;
        o--;
        i--;
    }
    printf("Final Result: %d\n", num[0]);
}

int main(){
    printf("Enter the Expression:");
    char str[256];
    fgets(str, sizeof(str), stdin);
    // printf("Input Expression size: %d\n", strlen(str));
    evaluate(str);
    return 0;
}