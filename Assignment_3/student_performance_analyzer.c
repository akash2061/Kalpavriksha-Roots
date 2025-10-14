#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SUBJECTS 3

typedef struct StudentData{
    int roll_no;
    char name[50];
    float marks[MAX_SUBJECTS];
} Data;

void takeInput(Data *student, int);
void displayStudent(Data *student, int);
float totalMarks(Data student);
float avgMarks(Data student);
void grades(Data student);
void displayRollNo(Data *student, int);

int main(){
    int student_count;
    scanf("%d", &student_count);
    getchar();
    Data student[student_count];
    takeInput(student, student_count);
    displayStudent(student, student_count);
    return 0;
}

void takeInput(Data students[], int student_count){
    for(int i = 0; i < student_count; i++){
        char input_string[100];
        fgets(input_string, sizeof(input_string) - 1, stdin);
        char *token = strtok(input_string, " ");
        if(atoi(token) <= 0){
            printf("Invalid Roll Number.\nRe-enter details for student %d\n", i+1);
            i--;
            continue;
        }
        students[i].roll_no = atoi(token);
        token = strtok(NULL, " ");
        if(token == NULL || strlen(token) == 0 || isalpha(token[0]) == 0){
            printf("Invalid Name.\nRe-enter details for student %d\n", i+1);
            i--;
            continue;
        }
        strcpy(students[i].name, token);
        int subject_count = 0;
        while((token = strtok(NULL, " ")) != NULL && subject_count != MAX_SUBJECTS){
            if(atof(token) < 0 || atof(token) > 100){
                printf("Invalid Marks.\nRe-enter details for student %d\n", i+1);
                i--;
                break;
            }
            students[i].marks[subject_count] = atof(token);
            subject_count++;
        }
    }
}

void displayStudent(Data student[], int student_count){
    for(int i = 0; i < student_count; i++){
        printf("\nRoll: %d\nName: %s\n", student[i].roll_no, student[i].name);
        printf("Total: %.2f\n", totalMarks(student[i]));
        printf("Average: %.2f\n", avgMarks(student[i]));
        grades(student[i]);
    }
    printf("List of Roll Numbers (via recursion): ");
    displayRollNo(student, student_count);
}

float totalMarks(Data student){
    float total_marks = 0;
    for (int j = 0; j < MAX_SUBJECTS; j++){
        total_marks += student.marks[j];
    }
    return total_marks;
}

float avgMarks(Data student){
    return (totalMarks(student) / (float) MAX_SUBJECTS);
}

void grades(Data student){
    float avg_marks = avgMarks(student);
    if(avg_marks >= 85){
        printf("Grade: A\n");
        printf("Performance: *****\n");
    }else if(avg_marks >= 70){
        printf("Grade: B\n");
        printf("Performance: ****\n");
    }else if(avg_marks >= 50){
        printf("Grade: C\n");
        printf("Performance: ***\n");
    }else if(avg_marks >= 35){
        printf("Grade: D\n");
        printf("Performance: **\n");
    }else{
        printf("Grade: F\n");
    }
    return;
}

void displayRollNo(Data students[], int total_students){
    if (total_students == 0){
        return;
    }

    int min_index = 0;
    for (int i = 1; i < total_students; i++) {
        if (students[i].roll_no < students[min_index].roll_no){
            min_index = i;
        }
    }

    printf("%d ", students[min_index].roll_no);

    Data sort_students = students[min_index];
    students[min_index] = students[total_students-1];
    students[total_students-1] = sort_students;

    displayRollNo(students, total_students - 1);
}