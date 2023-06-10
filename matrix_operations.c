#include "matrix_operations.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

//Reads command line arguments

int readCommandLineArguments(int argc, char **argv, char **inputFile1, char **inputFile2, int *MatrixSize)
{
    if (argc < 4)
    {
        printf("Insufficient command line arguments.\n");
        return -1;
    }

    *inputFile1 = argv[1];
    *inputFile2 = argv[2];
    *MatrixSize = atoi(argv[3]);

    if (*MatrixSize <= 0)
    {
        printf("Invalid matrix size. Please provide a positive integer.\n");
        return -1;
    }

    return 0;
}

//----------------------------------------------------------    Matrix Operations    ----------------------------------------------------------//

void fillMatrixWithZeros(int **matrix, int size, int row)
{
    if (row >= size)
    {
        return; // Base case: all rows have been filled
    }
    memset(matrix[row], 0, size * sizeof(int));
    fillMatrixWithZeros(matrix, size, row + 1); // Recursive call to fill the next row
}

// Allocates memory for a matrix of size x size

int **allocateMatrix(int size)
{
    int **matrix = (int **)malloc(size * sizeof(int *));
    for (int row = 0; row < size; row++)
    {
        matrix[row] = (int *)malloc(size * sizeof(int));
    }
    return matrix;
}

// Allocates memory for a matrix of size x size and fills it with random numbers


int **readMatrixFromFile(char *filename, int size)
{
    int **matrix = allocateMatrix(size);
    fillMatrixWithZeros(matrix, size, 0);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error: Failed to open file %s\n", filename);
        return NULL;
    }

    char buffer[100]; // Assuming a maximum line length of 100 characters

    int row = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        int col = 0;
        int length = strlen(buffer);

        if (buffer[length - 1] == '\n')
        {
            buffer[length - 1] = '\0';
        }

        char *token = strtok(buffer, " \t");

        while (token != NULL)
        {
            matrix[row][col] = atoi(token);
            col++;
            token = strtok(NULL, " \t");
        }

        row++;
    }
    fclose(file);
    return matrix;
}

// Frees memory allocated for a matrix

void freeMatrix(int **matrix, int size)
{
    for (int row = 0; row < size; row++)
    {
        free(matrix[row]);
    }
    free(matrix);
}

//mustiplies two matrices and stores the result in result matrix

void multiplyMatrices(int **result, int **matrix1, int **matrix2, int size)
{
    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            result[row][col] = 0;
            for (int k = 0; k < size; k++)
            {
                result[row][col] += matrix1[row][k] * matrix2[k][col];
            }
        }
    }
}

//compares two matrices and returns true if they are equal

bool compareMatrices(char *file1, char *file2, char *file3, int size)
{
    int **expectedMatrix = readMatrixFromFile(file3, size);
    int **matrixA = readMatrixFromFile(file1, size);
    int **matrixB = readMatrixFromFile(file2, size);

    int **multipliedMatrix = allocateMatrix(size);
    multiplyMatrices(multipliedMatrix, matrixA, matrixB, size);

    int row, col;
    bool equal = true;

    for (row = 0; row < size && equal; row++)
    {
        for (col = 0; col < size; col++)
        {
            if (expectedMatrix[row][col] != multipliedMatrix[row][col])
            {
                equal = false;
                break;
            }
        }
    }

    freeMatrix(expectedMatrix, size);
    freeMatrix(multipliedMatrix, size);
    freeMatrix(matrixA, size);
    freeMatrix(matrixB, size);

    return equal;
}

//----------------------------------------------------------    MPI Operations    ----------------------------------------------------------//

void printReducerRanks(int *reducerRanks, int numOfReducers)
{
    for (int i = 0; i < numOfReducers; i++)
    {
        printf("%d ", reducerRanks[i]);
    }
    printf("\n");
}
void printProcessorCount(int numOfProcess)
{
    printf("Processors: %d\n", numOfProcess);
}
void printReducerCount(int numOfReducers)
{
    printf("Reducers: %d\n", numOfReducers);
}
void printReducerChunkSize(int reducerChunkSize)
{
    printf("Reducer chunksize: %d\n", reducerChunkSize);
}
void populateMatricesFromFile(char *file1, char *file2, int size, int ***matrix1, int ***matrix2)
{
    *matrix1 = readMatrixFromFile(file1, size);

    *matrix2 = readMatrixFromFile(file2, size);
}
void printMasterDetails(int rank, char *machineName)
{
    printf("Master with process_id %d running on %s.\n", rank, machineName);
}


//sends the rows of the matrices to the mappers

void sendMatrixRowsToMappers(int rank, int Mappers, int chunkSize, int size, int **matrix1, int **matrix2)
{
    // Function to send matrix rows to mappers
    // Inputs:
    // - rank: current process rank
    // - Mappers: total number of mappers
    // - chunkSize: number of rows assigned to each mapper
    // - size: size of the matrix
    // - matrix1: pointer to the first matrix
    // - matrix2: pointer to the second matrix

    // Initialize a variable k with a value of 0
    int k = 0;
    for (int i = 1; i < Mappers + 1; i++)
    {
        // Iterate over the range [1, Mappers + 1) with the variable i
        // i represents the current mapper's index

        for (int j = 0; j < chunkSize; j++, k++)
        {
            // Iterate over the range [0, chunkSize) with the variable j
            // j represents the current row index within the assigned chunk

            // Send the value of k (current row index) to the mapper with rank i
            MPI_Send(&k, 1, MPI_INT, i, 1, MPI_COMM_WORLD);

            // Send the contents of the kth row of matrix1 to the mapper with rank i
            MPI_Send(matrix1[k], size, MPI_INT, i, 2, MPI_COMM_WORLD);

            // Send the contents of the kth row of matrix2 to the mapper with rank i
            MPI_Send(matrix2[k], size, MPI_INT, i, 3, MPI_COMM_WORLD);
        }

        // Print a message indicating the task map assigned to process i
        printf("Task Map Assigned to process %d.\n", i);
    }

}


void freeMasterResources(int **matrix1, int **matrix2, char *machineName, int Size)
{
    freeMatrix(matrix1, Size);
    freeMatrix(matrix2, Size);
    free(machineName);
}


void printReceivedTask(int rank, const char *machineName)
{
    printf("Process %d received task map on %s.\n", rank, machineName);
}


void receiveData(int rank, int size, int *row, int **matrix1, int **matrix2)
{
    // Function to receive data from the master process
    // Inputs:
    // - rank: current process rank
    // - size: size of the matrix
    // - row: pointer to store the received row index
    // - matrix1: pointer to store the received matrix1 row
    // - matrix2: pointer to store the received matrix2 row

    MPI_Status status;
    // Create an MPI_Status object to store status information about the received message

    MPI_Recv(row, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    // Receive a single integer (row index) from the master process (rank 0)
    // The received data is stored in the 'row' variable
    // The tag 1 is used to identify the message type
    // MPI_COMM_WORLD is the communicator representing all processes
    // The 'status' object will store information about the received message

    *matrix1 = (int *)malloc(size * sizeof(int));
    // Allocate memory for matrix1 row
    // The size of the row is 'size' multiplied by the size of an integer

    *matrix2 = (int *)malloc(size * sizeof(int));
    // Allocate memory for matrix2 row
    // The size of the row is 'size' multiplied by the size of an integer

    MPI_Recv(*matrix1, size, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
    // Receive 'size' number of integers (matrix1 row) from the master process (rank 0)
    // The received data is stored in the memory pointed to by 'matrix1'
    // The tag 2 is used to identify the message type
    // MPI_COMM_WORLD is the communicator representing all processes
    // The 'status' object will store information about the received message

    MPI_Recv(*matrix2, size, MPI_INT, 0, 3, MPI_COMM_WORLD, &status);
    // Receive 'size' number of integers (matrix2 row) from the master process (rank 0)
    // The received data is stored in the memory pointed to by 'matrix2'
    // The tag 3 is used to identify the message type
    // MPI_COMM_WORLD is the communicator representing all processes
    // The 'status' object will store information about the received message
}

//-----------------------------Mapper---------------------------------//

void sendMapperData(const MatrixKey *key, const MatrixValue *value)
{
    // Function to send mapper data to the master process
    // Inputs:
    // - key: pointer to the MatrixKey struct containing mapper data
    // - value: pointer to the MatrixValue struct containing mapper data

    MPI_Send(key, sizeof(MatrixKey), MPI_BYTE, 0, 10, MPI_COMM_WORLD);
    // Send the data pointed to by 'key' to the master process (rank 0)
    // The size of the data is determined by sizeof(MatrixKey)
    // MPI_BYTE is the datatype of the data being sent
    // The tag 10 is used to identify the message type
    // MPI_COMM_WORLD is the communicator representing all processes

    MPI_Send(value, sizeof(MatrixValue), MPI_BYTE, 0, 20, MPI_COMM_WORLD);
    // Send the data pointed to by 'value' to the master process (rank 0)
    // The size of the data is determined by sizeof(MatrixValue)
    // MPI_BYTE is the datatype of the data being sent
    // The tag 20 is used to identify the message type
    // MPI_COMM_WORLD is the communicator representing all processes
}

//--------------------------------------------------------------------//

void freeData(int *matrix1, int *matrix2)
{
    free(matrix1);
    free(matrix2);
}
void printCompletedTask(int rank, const char *machineName)
{
    printf("Process %d has completed task map on %s.\n", rank, machineName);
}

///----------------------------------------------------------------------   //


void processTaskMap(int rank, int dropout, int chunkSize, int size)
{
    // Function to process the mapping task for a specific rank
    // Inputs:
    // - rank: rank of the current process
    // - dropout: number of processes to be ignored in mapping task
    // - chunkSize: number of rows to process
    // - size: size of the matrices

    if (rank != 0 && rank < dropout)
    {
        // Check if the current process rank is not 0 and less than the dropout value

        char *machineName = malloc(MPI_MAX_PROCESSOR_NAME * sizeof(char));
        // Allocate memory to store the machine name where the process is running
        int nameLength;
        MPI_Get_processor_name(machineName, &nameLength);
        // Get the name of the machine where the process is running and store it in machineName
        printReceivedTask(rank, machineName);
        // Print a message indicating that the task has been received by the process
        MPI_Status status;

        for (int ind = 0; ind < chunkSize; ind++)
        {
            // Loop over the chunkSize, which represents the number of rows to process

            int row = 0;
            int *matrixA = NULL;
            int *matrixB = NULL;
            // Declare variables to store the row number, matrix A, and matrix B

            receiveData(rank, size, &row, &matrixA, &matrixB);
            // Receive data from the master process (rank 0) into row, matrixA, and matrixB

            int j, k;
            for (j = 0, k = 0; j < size * size; j++, k = (j / size))
            {
                // Loop over the elements of matrix A

                MatrixKey key;
                MatrixValue value;
                // Declare variables to store the key and value for mapping

                key.k = j % size;
                key.i = row;
                // Set the key values based on the current indices
                value.mat = '1';
                value.j = k;
                value.val = matrixA[k];
                // Set the value attributes based on the current indices and matrixA

                sendMapperData(&key, &value);
                // Send the key-value pair to the master process
            }

            int i = 0;
            while (i < size * size)
            {
                // Loop over the elements of matrix B

                int k = i / size;
                int j = i % size;
                // Calculate the indices based on the current iteration

                MatrixKey key;
                MatrixValue value;
                // Declare variables to store the key and value for mapping

                key.i = j;
                key.k = k;
                // Set the key values based on the current indices
                value.mat = '2';
                value.j = row;
                value.val = matrixB[k];
                // Set the value attributes based on the current indices and matrixB

                sendMapperData(&key, &value);
                // Send the key-value pair to the master process

                i++;
            }

            freeData(matrixA, matrixB);
            // Free the memory allocated for matrixA and matrixB
        }

        printCompletedTask(rank, machineName);
        // Print a message indicating that the task has been completed by the process
        free(machineName);
        // Free the memory allocated for machineName
    }
}

//--------------------------------------------------------------------//

void receiveMapperData(int source, MatrixKey *key, MatrixValue *value)
{
    // Function to receive data from a specific source process
    // Inputs:
    // - source: rank of the source process
    // - key: pointer to MatrixKey struct to store the received key
    // - value: pointer to MatrixValue struct to store the received value

    MPI_Recv(key, sizeof(MatrixKey), MPI_BYTE, source, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // Receive the MatrixKey struct from the source process
    // - key: pointer to the memory location where the received key will be stored
    // - sizeof(MatrixKey): size of the MatrixKey struct in bytes
    // - MPI_BYTE: data type of the received message
    // - source: rank of the source process
    // - 10: message tag associated with the received message
    // - MPI_COMM_WORLD: communicator that identifies the group of processes
    // - MPI_STATUS_IGNORE: ignore the status information of the received message

    MPI_Recv(value, sizeof(MatrixValue), MPI_BYTE, source, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // Receive the MatrixValue struct from the source process
    // - value: pointer to the memory location where the received value will be stored
    // - sizeof(MatrixValue): size of the MatrixValue struct in bytes
    // - MPI_BYTE: data type of the received message
    // - source: rank of the source process
    // - 20: message tag associated with the received message
    // - MPI_COMM_WORLD: communicator that identifies the group of processes
    // - MPI_STATUS_IGNORE: ignore the status information of the received message
}


//--------------------------------------------------------------------//

void assignReduceTask(int rank, int *reducerRanks, int reducerChunkSize, int size, MatrixKey *keys, MatrixValue *values)
{
    // Function to assign reduce tasks to specific processes
    // Inputs:
    // - rank: rank of the current process
    // - reducerRanks: array of reducer process ranks
    // - reducerChunkSize: number of reduce tasks assigned to each reducer process
    // - size: size of the matrices (size x size)
    // - keys: array of MatrixKey structs
    // - values: array of MatrixValue structs

    int t = 0;
    int counter = 0;
    // Initialize variables to track reducer ranks and task counter

    for (int i = 0; i < size * size; i++)
    {
        int row = i / size;
        int col = i % size;
        // Compute row and column indices from the current index

        MatrixKey key;
        key.i = row;
        key.k = col;
        // Create a MatrixKey struct with the current row and column indices

        if (counter == reducerChunkSize)
        {
            printf("Task Reduce Assigned to process %d.\n", reducerRanks[t]);
            // Print a message indicating the assignment of a reduce task to a specific process
            t++;
            // Move to the next reducer process
            counter = 0;
            // Reset the task counter
        }

        MPI_Send(&key, sizeof(MatrixKey), MPI_BYTE, reducerRanks[t], 10, MPI_COMM_WORLD);
        // Send the MatrixKey struct to the current reducer process
        // - &key: pointer to the MatrixKey struct
        // - sizeof(MatrixKey): size of the MatrixKey struct in bytes
        // - MPI_BYTE: data type of the message
        // - reducerRanks[t]: rank of the current reducer process
        // - 10: message tag associated with the message
        // - MPI_COMM_WORLD: communicator that identifies the group of processes

        for (int k = 0; k < size * size * size * 2; k++)
        {
            MatrixValue value = values[k];
            // Get the current MatrixValue from the values array

            if (keys[k].i == row && keys[k].k == col)
            {
                MPI_Send(&value, sizeof(MatrixValue), MPI_BYTE, reducerRanks[t], 20, MPI_COMM_WORLD);
                // Send the MatrixValue to the current reducer process if the key matches
            }
        }

        counter++;
        // Increment the task counter
    }

    printf("Task Reduce Assigned to process %d.\n", reducerRanks[t]);
    // Print a message indicating the assignment of a reduce task to the last reducer process
}

//--------------------------------------------------------------------//


int *initializeReducerRanks(int Reducers, int totalproc)
{
    int *reducerRanks = (int *)malloc(Reducers * sizeof(int));
    for (int i = 0; i < Reducers; i++)
    {
        reducerRanks[i] = i + 1;
    }
    return reducerRanks;
}

//--------------------------------------------------------------------//

void validateMapperConfiguration(int Size, int Mappers, int totalproc, int *dropout)
{
    if (Size * Size * 2 * Mappers > totalproc - 1)
    {
        printf("Error: Number of processes is not sufficient to perform the task.\n");
        printf("Number of processes required: %d\n", Size * Size * 2 * Mappers + 1);
        printf("Number of processes available: %d\n", totalproc);
        *dropout = 1;
    }
}

//--------------------------------------------------------------------//

void writeResultToFile(int Rank, int Size, int **outputarr, char *File1, char *File2)
{
    // Function to write the result of matrix multiplication to a file
    // Inputs:
    // - Rank: rank of the current process
    // - Size: size of the matrices (Size x Size)
    // - outputarr: 2D array containing the result matrix
    // - File1: name of the first input file
    // - File2: name of the second input file

    if (Rank == 0)
    {
        // Execute the following code only for the root process (Rank 0)

        FILE *outp;
        outp = fopen("Output.txt", "w");
        // Open the "Output.txt" file in write mode

        if (outp == NULL)
        {
            printf("Error!");
            exit(1);
        }
        // Check if the file was opened successfully, and if not, print an error message and exit the program

        for (int i = 0; i < Size * Size; i++)
        {
            ReducerKeyValue KeyValue;
            MPI_Recv(&KeyValue, sizeof(ReducerKeyValue), MPI_BYTE, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Receive a ReducerKeyValue struct from any source process
            // - &KeyValue: pointer to the ReducerKeyValue struct
            // - sizeof(ReducerKeyValue): size of the ReducerKeyValue struct in bytes
            // - MPI_BYTE: data type of the message
            // - MPI_ANY_SOURCE: accept messages from any source process
            // - 5: message tag associated with the message
            // - MPI_COMM_WORLD: communicator that identifies the group of processes

            outputarr[KeyValue.row][KeyValue.col] = KeyValue.value;
            // Store the received value in the appropriate position of the outputarr matrix
        }

        printf("\nJob has been Completed");

        for (int i = 0; i < Size; i++)
        {
            for (int j = 0; j < Size; j++)
            {
                fprintf(outp, "%d ", outputarr[i][j]);
                // Write each element of the outputarr matrix to the file
            }
            fprintf(outp, "\n");
            // Write a new line character to separate rows in the file
        }

        fclose(outp);
        // Close the file

        printf("\nMatrix Comparison Function Returned: ");
        if (compareMatrices(File1, File2, "Output.txt", Size))
        {
            printf("True\n");
        }
        else
        {
            printf("False");
        }
        // Call the compareMatrices function to compare the generated output with the expected result
    }
}


//--------------------------------------------------------------------//

void performReduceMap(int Rank, int Size, int ReducerChunkSize)
{
    // Function to perform the reduce map operation
    // Inputs:
    // - Rank: rank of the current process
    // - Size: size of the matrices (Size x Size)
    // - ReducerChunkSize: number of elements to process in each iteration

    int counter = 0;
    char MachineName[MPI_MAX_PROCESSOR_NAME];
    int Len;
    MPI_Get_processor_name(MachineName, &Len);
    // Declare variables for counting and storing the machine name of the current process
    // MPI_Get_processor_name is used to obtain the name of the processor running the current process

    int n = 0;
    // Initialize a variable n to track the index of received values

    for (int i = 0, counter = 0; i < Size * Size && counter < ReducerChunkSize; i++, counter++)
    {
        // Loop over the total number of elements in the matrices and process only a chunk of ReducerChunkSize

        MatrixKey Key;
        MatrixValue Values[Size * 2];
        // Declare MatrixKey and MatrixValue arrays to store received keys and values

        MPI_Recv(&Key, sizeof(Key), MPI_BYTE, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Receive a MatrixKey struct from the root process
        // - &Key: pointer to the MatrixKey struct
        // - sizeof(Key): size of the MatrixKey struct in bytes
        // - MPI_BYTE: data type of the message
        // - 0: rank of the root process
        // - 10: message tag associated with the message
        // - MPI_COMM_WORLD: communicator that identifies the group of processes
        // - MPI_STATUS_IGNORE: ignore the status of the receive operation

        for (int k = 0; k < Size * 2; k++, n++)
        {
            MatrixValue Value;
            MPI_Recv(&Value, sizeof(Value), MPI_BYTE, 0, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Receive a MatrixValue struct from the root process
            // - &Value: pointer to the MatrixValue struct
            // - sizeof(Value): size of the MatrixValue struct in bytes
            // - MPI_BYTE: data type of the message
            // - 0: rank of the root process
            // - 20: message tag associated with the message
            // - MPI_COMM_WORLD: communicator that identifies the group of processes
            // - MPI_STATUS_IGNORE: ignore the status of the receive operation

            Values[k] = Value;
            // Store the received value in the Values array
        }

        int val = 0;
        int m = 0;
        while (m < Size)
        {
            int temp = 1;
            int n = 0;
            while (n < Size * 2)
            {
                if (Values[n].j == m)
                {
                    temp *= Values[n].val;
                }
                n++;
            }
            val += temp;
            m++;
        }
        // Calculate the reduced value for the given key and values

        ReducerKeyValue KeyValue;
        KeyValue.row = Key.i;
        KeyValue.col = Key.k;
        KeyValue.value = val;
        // Create a ReducerKeyValue struct and assign the calculated values

        MPI_Send(&KeyValue, sizeof(ReducerKeyValue), MPI_BYTE, 0, 5, MPI_COMM_WORLD);
        // Send the ReducerKeyValue struct to the root process

    }
    printf("\nProcess %d has completed Reduce map on %s.\n", Rank, MachineName);
}


//-------------------------------------------------------------------ENd of Functions---------------------------------------------------//
