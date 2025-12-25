#include <stdio.h>
#include <ctype.h>
#include <string.h>

int numDigits(const char *str, int *position){
    int digits = 0;
    while (isdigit(str[*position])){
        digits = digits * 10 + (str[*position] - '0');
        (*position)++;
    }
    (*position)--;
    return digits;
}

void evaluate(const char *str){
    int position = 1;
    int digits[128], digit_count = 0;
    char operators[128], operator_count = 0;
    int start_pos = 0;
    if (str[0] == '-'){
        digits[digit_count++] = -numDigits(str, &position);
        start_pos = position + 1;
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
            digits[digit_count++] = numDigits(str, &i);
        }
        else if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/'){
            if (i == 0 && str[i] == '-'){
                continue;
            }
            if (str[i + 1] == '+' || str[i + 1] == '-' || str[i + 1] == '*' || str[i + 1] == '/'){
                printf("Test ERROR: Invalid Expression. %d\n", i);
                return;
            }
            operators[operator_count++] = str[i];
        }
        else{
            printf("Global ERROR: Invalid Expression. %d\n", i);
            return;
        }
    }
    for (int i = 0; i < operator_count; i++){
        if (operators[i] == '*' || operators[i] == '/'){
            if (operators[i] == '*'){
                digits[i] = digits[i] * digits[i + 1];
            }
            else{
                if (digits[i + 1] == 0){
                    printf("ERROR: Division by zero.\n");
                    return;
                }
                digits[i] = digits[i] / digits[i + 1];
            }
            for (int j = i + 1; j < digit_count - 1; j++){
                digits[j] = digits[j + 1];
            }
            for (int j = i; j < operator_count - 1; j++){
                operators[j] = operators[j + 1];
            }
            digit_count--;
            operator_count--;
            i--;
        }
    }
    for (int i = 0; i < operator_count; i++){
        if (operators[i] == '+'){
            digits[i] = digits[i] + digits[i + 1];
        }
        else if (operators[i] == '-'){
            digits[i] = digits[i] - digits[i + 1];
        }
        for (int j = i + 1; j < digit_count - 1; j++){
            digits[j] = digits[j + 1];
        }
        for (int j = i; j < operator_count - 1; j++){
            operators[j] = operators[j + 1];
        }
        digit_count--;
        operator_count--;
        i--;
    }
    printf("Final Result: %d\n", digits[0]);
}

int main(){
    printf("Enter the Expression:");
    char str[256];
    fgets(str, sizeof(str), stdin);
    evaluate(str);
    return 0;
}
