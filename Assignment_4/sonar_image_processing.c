#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_MATRIX_SIZE 2
#define MAX_MATRIX_SIZE 10

void swap(unsigned short int*, unsigned short int*);
void generateMatrix(unsigned short int**, int);
void displayMatrix(unsigned short int**, int);
void clockwiseRotate(unsigned short int**, int);
void smoothingFilter(unsigned short int**, int);
void leftShift(unsigned short int**, int);
void clearUpperByte(unsigned short int**, int);
void freeMatrix(unsigned short int**, int);

int main(){
    int matrixSize;

    do{
        scanf("%d", &matrixSize);
        if(matrixSize < MIN_MATRIX_SIZE || matrixSize > MAX_MATRIX_SIZE){
            printf("ERROR: Invalid Matrix Size.\nRe-enter Matrix Size [%d-%d]: ", MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
        }
        getchar();
    }while(matrixSize < MIN_MATRIX_SIZE || matrixSize > MAX_MATRIX_SIZE);

    unsigned short int **matrix = (unsigned short int **)malloc(matrixSize * sizeof(unsigned short int *));

    generateMatrix(matrix, matrixSize);
    printf("\nOriginal Matrix:\n");
    displayMatrix(matrix, matrixSize);

    clockwiseRotate(matrix, matrixSize);
    printf("\nRotated Matrix:\n");
    displayMatrix(matrix, matrixSize);

    leftShift(matrix, matrixSize);
    smoothingFilter(matrix, matrixSize);
    clearUpperByte(matrix, matrixSize);

    printf("\nFinal Matrix:\n");
    displayMatrix(matrix, matrixSize);

    freeMatrix(matrix, matrixSize);
    return 0;
}

void generateMatrix(unsigned short int **matrix, int matrixSize){
    srand(time(0));

    for(int i = 0; i < matrixSize; i++){
        *(matrix + i) = (unsigned short int *)malloc(matrixSize * sizeof(unsigned short int));
        if(!*(matrix + i)){
            printf("Memory allocation failed! for %d index.\n", i);
            freeMatrix(matrix, i);
            exit(1);
        }
    }

    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            *(*(matrix + i) + j) = rand() % 256;
        }
    }
}

void displayMatrix(unsigned short int **matrix, int matrixSize){
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            printf("%3d ", *(*(matrix + i) + j));
        }
        printf("\n");
    }
}

void clockwiseRotate(unsigned short int **matrix, int matrixSize){
    for(int i = 0; i < matrixSize; i++){
        for(int j = i + 1; j < matrixSize; j++){
            short int *intensity_a = *(matrix + i) + j;
            short int *intensity_b = *(matrix + j) + i;
            swap(intensity_a, intensity_b);
        }
    }

    for(int i = 0; i < matrixSize; i++){
        short int *left = *(matrix + i);
        short int *right = *(matrix + i) + matrixSize - 1;
        while (left < right)
        {
            swap(left, right);
            left++;
            right--;
        }
    }
}

void smoothingFilter(unsigned short int **matrix, int matrixSize){
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            int sum = 0;
            int divisor = 0;
            for(int row_offset = -1; row_offset <= 1; row_offset++){
                for(int col_offset = -1; col_offset <= 1; col_offset++){
                    int nighbor_rows = i + row_offset;
                    int nighbor_cols = j + col_offset;

                    if(nighbor_rows >= 0 && nighbor_rows < matrixSize && nighbor_cols >= 0 && nighbor_cols < matrixSize){
                        // !Extract upper 8 bits without modifying original value of matrix. (right shift)
                        // !Bit-Mapping: [Original Value : 8 bits] [Filtered Value : 8 bits] (16 bits total)
                        // !EXAMPLE: 1111 1011 0000 0100 (64260) -> 0000 0000 1111 1011 (251) [unsigned short int]
                        int upper_bits = *(*(matrix + nighbor_rows) + nighbor_cols) >> 8;
                        sum += upper_bits;
                        divisor++;
                    }
                }
            }
            sum /= divisor;
            *(*(matrix + i) + j) += sum;
        }
    }
}

void swap(unsigned short int *intensity_a, unsigned short int *intensity_b){
    unsigned short int temporary_intensity = *intensity_a;
    *intensity_a = *intensity_b;
    *intensity_b = temporary_intensity;
}

void freeMatrix(unsigned short int **matrix, int matrixSize){
    for(int i = 0; i < matrixSize; i++){
        free(*(matrix + i));
    }
    free(matrix);
}

void clearUpperByte(unsigned short int **matrix, int matrixSize){
    // !EXAMPLE: 1111 1111 1111 1111 (65535) -> 0000 0000 1111 1111 (255) [unsigned short int]
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            *(*(matrix + i) + j) <<= 8;
            *(*(matrix + i) + j) >>= 8;
        }
    }
}

void leftShift(unsigned short int **matrix, int matrixSize){
    // !EXAMPLE: 0000 0000 1111 1111 (255) -> 1111 1111 0000 0000 (65280) [unsigned short int]
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            *(*(matrix + i) + j) <<= 8;
        }
    }
}
