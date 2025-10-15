#include <stdio.h>
#include <string.h>

#define FILENAME "users.txt"
#define MAX_USERS 100

typedef struct User{
    int id;
    char name[50];
    int age;
} User;

void menu();
void create();
void read();
void update();
void delete();
int uniqueID();
int userCount(User users[]);
void clearInputBuffer();

int main(){
    int choice;
    do {
        menu();
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1){
            printf("Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        switch (choice){
        case 1:
            create();
            break;
        case 2:
            read();
            break;
        case 3:
            update();
            break;
        case 4:
            delete();
            break;
        case 5:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice! Try again.\n");
        }
    } while(choice != 5);
    return 0;
}

void menu(){
    printf("\n=== MENU ===\n");
    printf("1. Create User\n");
    printf("2. Read All Users\n");
    printf("3. Update User\n");
    printf("4. Delete User\n");
    printf("5. Exit\n");
    printf("============\n");
}

int userCount(User users[]){
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL){
        return 0;
    }

    int count = 0;
    while (fscanf(fp, "%d %s %d", &users[count].id, users[count].name, &users[count].age) == 3){
        count++;
    }
    fclose(fp);
    return count;
}

int uniqueID(){
    User users[MAX_USERS];
    int count = userCount(users);
    int maxID = 0;

    for (int i = 0; i < count; i++){
        if (users[i].id > maxID){
            maxID = users[i].id;
        }
    }
    return maxID + 1;
}

void create(){
    User u;
    u.id = uniqueID();

    printf("Assigned User ID: %d\n", u.id);
    printf("Enter Name: ");
    if (fgets(u.name, sizeof(u.name), stdin) != NULL){
        u.name[strcspn(u.name, "\n")] = 0;
    }

    if (strlen(u.name) == 0){
        printf("Error: Name cannot be empty.\n");
        return;
    }

    printf("Enter Age: ");
    while (scanf("%d", &u.age) != 1 || u.age < 0 || u.age > 150){
        printf("Invalid age! Please enter a valid age (0-150): ");
        clearInputBuffer();
    }
    clearInputBuffer();

    FILE *fp = fopen(FILENAME, "a");
    if (fp == NULL){
        printf("Error opening file.\n");
        return;
    }

    fprintf(fp, "%d %s %d\n", u.id, u.name, u.age);
    fclose(fp);
    printf("User added successfully.\n");
}

void read(){
    User u[MAX_USERS];
    int count = userCount(u);
    if (count == 0){
        printf("No users found.\n");
        return;
    }
    printf("\n--- User Records ---\n");
    printf("%-5s %-20s %-5s\n", "ID", "Name", "Age");
    printf("%-5s %-20s %-5s\n", "---", "--------------------", "---");
    for (int i = 0; i < count; i++){
        printf("%-5d %-20s %-5d\n", u[i].id, u[i].name, u[i].age);
    }
}

void update(){
    int id;
    printf("Enter ID of user to update: ");
    scanf("%d", &id);
    clearInputBuffer();

    User users[MAX_USERS];
    int count = userCount(users);

    int found = 0;
    for (int i = 0; i < count; i++){
        if (users[i].id == id){
            printf("Enter new name: ");
            if (fgets(users[i].name, sizeof(users[i].name), stdin) != NULL){
                users[i].name[strcspn(users[i].name, "\n")] = 0;
            }
            printf("Enter new age: ");
            while (scanf("%d", &users[i].age) != 1 || users[i].age < 0 || users[i].age > 150){
                printf("Invalid age! Please enter a valid age (0-150): ");
                clearInputBuffer();
            }
            clearInputBuffer();
            found = 1;
            break;
        }
    }

    if (!found){
        printf("User with ID %d not found.\n", id);
        return;
    }

    FILE *fp = fopen(FILENAME, "w");
    for (int i = 0; i < count; i++){
        fprintf(fp, "%d %s %d\n", users[i].id, users[i].name, users[i].age);
    }
    fclose(fp);
    printf("User updated successfully.\n");
}

void delete(){
    int id;
    printf("Enter ID of user to delete: ");
    scanf("%d", &id);
    clearInputBuffer();

    User users[MAX_USERS];
    int count = userCount(users);

    FILE *fp = fopen(FILENAME, "w");
    int deleted = 0;
    for (int i = 0; i < count; i++){
        if (users[i].id == id){
            deleted = 1;
            continue;
        }
        fprintf(fp, "%d %s %d\n", users[i].id, users[i].name, users[i].age);
    }
    fclose(fp);

    if (deleted){
        printf("User deleted successfully.\n");
    }
    else{
        printf("User with ID %d not found.\n", id);
    }
}

void clearInputBuffer(){
    int input_buffer;
    while ((input_buffer = getchar()) != '\n' && input_buffer != EOF);
}
