#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_SIZE 100
#define FILENAME "data.txt"

int getValidInteger();
int readInputArray(int *);
void displayArray(int *, int);
void writeArrayToFile(int *, int);
int readArrayFromFile(int *);
void sortArray(int *, int);
void parentProcess(int *, int);
void childProcess();
void initiateFileIPC();

int main()
{
    initiateFileIPC();
    return 0;
}

int getValidInteger()
{
    int number;
    while (1)
    {
        if (scanf("%d", &number) != 1)
        {
            printf("Invalid input. Please enter again: ");
            while (getchar() != '\n')
                ;
        }
        else
        {
            if (getchar() != '\n')
            {
                printf("Invalid input. Please enter again: ");
                while (getchar() != '\n')
                    ;
            }
            else
            {
                return number;
            }
        }
    }
}

int readInputArray(int *arr)
{
    int size;
    printf("Enter number of elements: ");
    size = getValidInteger();
    if (size <= 0 || size > MAX_SIZE)
    {
        printf("Invalid size. Please enter between 1 and %d\n", MAX_SIZE);
        return -1;
    }
    printf("Enter %d elements:\n", size);
    for (int i = 0; i < size; i++)
    {
        arr[i] = getValidInteger();
    }
    return size;
}

void displayArray(int *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void writeArrayToFile(int *arr, int size)
{
    FILE *file = fopen(FILENAME, "w");
    if (!file)
    {
        printf("Error opening file.\n");
        return;
    }
    fprintf(file, "%d\n", size);
    for (int i = 0; i < size; i++)
    {
        fprintf(file, "%d ", arr[i]);
    }
    fclose(file);
}

int readArrayFromFile(int *arr)
{
    FILE *file = fopen(FILENAME, "r");
    if (!file)
    {
        printf("Error opening file.\n");
        return 0;
    }
    int size;
    if (fscanf(file, "%d", &size) != 1)
    {
        printf("Error reading size from file.\n");
        fclose(file);
        return 0;
    }

    if (size <= 0 || size > MAX_SIZE)
    {
        printf("Error: Invalid array size %d read from file. Must be between 1 and %d\n", size, MAX_SIZE);
        fclose(file);
        return 0;
    }

    for (int i = 0; i < size; i++)
    {
        if (fscanf(file, "%d", &arr[i]) != 1)
        {
            printf("Error reading array element %d from file.\n", i);
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    return size;
}

void sortArray(int *arr, int size)
{
    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void parentProcess(int *array, int size)
{
    printf("\n--- Process 1 - Parent ---\n");
    printf("Array before sorting: ");
    displayArray(array, size);
    writeArrayToFile(array, size);
    printf("Data written to file: %s\n", FILENAME);
}

void childProcess()
{
    printf("\n--- Process 2 - Child ---\n");
    int childArray[MAX_SIZE];
    int childSize = readArrayFromFile(childArray);
    printf("Child process sorting the array...\n ");
    sortArray(childArray, childSize);
    writeArrayToFile(childArray, childSize);
    printf("\nSorted data written back to file.\n");
}

void initiateFileIPC()
{
    int array[MAX_SIZE];
    int size;

    printf("\n============================================\n");
    printf("File-Based IPC Mechanism");
    printf("\n============================================\n");
    size = readInputArray(array);
    if (size == -1)
    {
        return;
    }

    parentProcess(array, size);

    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork failed.\n");
        return;
    }
    else if (pid == 0)
    {
        childProcess();
        exit(0);
    }
    else
    {
        wait(NULL);
        printf("\n--- Process 1 - Parent ---\n");
        int sortedArray[MAX_SIZE];
        int sortedSize = readArrayFromFile(sortedArray);
        printf("Array after sorting: ");
        displayArray(sortedArray, sortedSize);
    }

    printf("\n============================================\n");
    printf("File-Based IPC Completed Successfully");
    printf("\n============================================\n");
}
