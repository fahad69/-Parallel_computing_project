// ----------------------------------------------
// Included Libraries and Header Guard Directive
// ----------------------------------------------

#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H


// ---------------------------------
// Struct Definitions
// ---------------------------------

typedef struct {
    int i;
    int k;
} MatrixKey;

typedef struct {
    char mat;
    int j;
    int val;
} MatrixValue;

typedef struct {
    int row;
    int col;
    int value;
} ReducerKeyValue;


// ---------------------------------
// Function Declarations
// ---------------------------------

int readCommandLineArguments(int argc, char** argv, char** inputFile1, char** inputFile2, int* MatrixSize);
void fillMatrixWithZeros(int** matrix, int size, int row);
int** readMatrixFromFile(char* filename, int size);
int** allocateMatrix(int size);
void freeMatrix(int** matrix, int size);
void multiplyMatrices(int** result, int** matrix1, int** matrix2, int size);
bool compareMatrices(char* file1, char* file2, char* file3, int size);
void printReducerRanks(int* reducerRanks, int numOfReducers);
void printProcessorCount(int numOfProcess);
void printReducerCount(int numOfReducers);
void printReducerChunkSize(int reducerChunkSize);
void populateMatricesFromFile(char* file1, char* file2, int size, int*** matrix1, int*** matrix2);
void printMasterDetails(int rank, char* machineName);
void sendMatrixRowsToMappers(int rank, int Mappers, int chunkSize, int size, int** matrix1, int** matrix2);
void freeMasterResources(int** matrix1, int** matrix2, char* machineName, int Size);
void printReceivedTask(int rank, const char* machineName);
void receiveData(int rank, int size, int* row, int** matrix1, int** matrix2);
void sendMapperData(const MatrixKey* key, const MatrixValue* value);
void freeData(int* matrix1, int* matrix2);
void printCompletedTask(int rank, const char* machineName);
void processTaskMap(int rank, int dropout, int chunkSize, int size);
void receiveMapperData(int source, MatrixKey* key, MatrixValue* value);
void assignReduceTask(int rank, int* reducerRanks, int reducerChunkSize, int size, MatrixKey* keys, MatrixValue* values);
int* initializeReducerRanks(int Reducers, int totalproc);
void validateMapperConfiguration(int Size, int Mappers, int totalproc, int* dropout);
void writeResultToFile(int Rank, int Size, int** outputarr, char* File1, char* File2);
void performReduceMap(int Rank, int Size, int ReducerChunkSize);


#endif
