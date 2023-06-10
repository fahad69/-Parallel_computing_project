#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 16

int main() {
    // Seed the random number generator
    srand(time(NULL));

    // Initialize the matrices
    int ** A = (int**)malloc(N * sizeof(int*));
    int** B = (int**)malloc(N * sizeof(int*));

    for (int i = 0; i < N; i++) {
        A[i] = (int*)malloc(N * sizeof(int));
        B[i] = (int*)malloc(N * sizeof(int));
    }

    // Generate random values for A and B
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    // Save the matrices to files
    FILE* fileA = fopen("matrixA.txt", "w");
    FILE* fileB = fopen("matrixB.txt", "w");
    

    if (fileA != NULL && fileB != NULL) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                    
                    fprintf(fileA, "%d ", A[i][j]);
                    fprintf(fileB, "%d ", B[i][j]);
                    
            }
            fprintf(fileA, "\n");
            fprintf(fileB, "\n");

        }
        fclose(fileA);
        fclose(fileB);

    } else {
        printf("Error: Unable to open file\n");
    }

    // Deallocate memory
    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
  
    }

    free(A);
    free(B);


    return 0;
}
