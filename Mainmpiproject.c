#include "matrix_operations.h"
#include <mpi.h>

int main(int argc, char **argv)
{
    // -----------------------
    // Command-line Arguments
    // -----------------------

    char *inputFile1;
    char *inputFile2;
    int MatrixSize;

    if (readCommandLineArguments(argc, argv, &inputFile1, &inputFile2, &MatrixSize) != 0)  // read command line arguments
    {
        return -1;
    }

    // -----------------------
    // MPI Initialization
    // -----------------------

    int rank = -1, numOfProcesses = -1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numOfProcesses);

    // -----------------------
    // Process Setup
    // -----------------------

    int processSize = numOfProcesses;
    int Mappers = numOfProcesses - 1;
    int dropout = 100000;

    if (!(MatrixSize % Mappers == 0 && Mappers != 1) || numOfProcesses == 1)
    {
        if (numOfProcesses > 1)
        {
            while (MatrixSize % Mappers != 0)
            {
                dropout = numOfProcesses - 1;
                --numOfProcesses;
                Mappers = numOfProcesses - 1;
            }
        }
        else
        {
            exit(0);
        }
    }

    int Splits = MatrixSize / Mappers;
    int Reducers = Mappers / 2;

    if (Mappers == 1)
    {
        Reducers = 1;
    }

    // -----------------------
    // Reducer Initialization
    // -----------------------

    int reducerSplits = MatrixSize * MatrixSize / Reducers;
    int *dynamicReducers = initializeReducerRanks(Reducers, numOfProcesses);    // initialize reducer ranks

    // -----------------------
    // Master Section
    // -----------------------

    if (rank == 0)
    {
        int **matrix1;
        int **matrix2;
        populateMatricesFromFile(inputFile1, inputFile2, MatrixSize, &matrix1, &matrix2);   // populate matrices from files
        char *machineName = malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME);
        int l;
        MPI_Get_processor_name(machineName, &l);
        printMasterDetails(rank, machineName);
        sendMatrixRowsToMappers(rank, Mappers, Splits, MatrixSize, matrix1, matrix2);  // send matrix rows to mappers
        freeMasterResources(matrix1, matrix2, machineName, MatrixSize);   // free master resources
    }

    // -----------------------
    // Barrier Synchronization
    // -----------------------

    MPI_Barrier(MPI_COMM_WORLD);

    // -----------------------
    // Mapper Tasks
    // -----------------------

    if (rank != 0 && rank < dropout)
    {
        processTaskMap(rank, dropout, Splits, MatrixSize);
    }

    // -----------------------
    // Barrier Synchronization
    // -----------------------

    MPI_Barrier(MPI_COMM_WORLD);

    // -----------------------
    // Master Receives Mapper Data
    // -----------------------

    if (rank == 0)
    {
        MatrixKey keys[MatrixSize * MatrixSize * MatrixSize * 2];          // initialize keys and values
        MatrixValue values[MatrixSize * MatrixSize * MatrixSize * 2];     // initialize keys and values
        int i = 0;
        int j = 1, k = 0, n = 0;

        do
        {
            MatrixKey key;
            MatrixValue value;
            receiveMapperData(j, &key, &value);  // receive mapper data
            keys[i] = key;
            values[i] = value;
            i++;
            n++;
            if (n == MatrixSize * MatrixSize * 2)
            {
                n = 0;
                k++;

                if (k == Splits)
                {
                    k = 0;
                    j++;
                }
            }
        } while (j < Mappers + 1 && k < Splits && n < MatrixSize * MatrixSize * 2);  // loop through all mappers

        assignReduceTask(rank, dynamicReducers, reducerSplits, MatrixSize, keys, values);   // assign reduce task
    }

    // -----------------------
    // Barrier Synchronization
    // -----------------------

    MPI_Barrier(MPI_COMM_WORLD);

    // -----------------------
    // Perform Reduce Tasks
    // -----------------------

    int indexofred = 0;
    while (indexofred < Reducers)  // loop through all reducers
    { 
        if (rank == dynamicReducers[indexofred])        
        {
            performReduceMap(rank, MatrixSize, reducerSplits);   // perform reduce task
            free(dynamicReducers);
        }
        indexofred++;  // increment index
    }

    // -----------------------
    // Barrier Synchronization
    // -----------------------

    MPI_Barrier(MPI_COMM_WORLD);

    // -----------------------
    // Write Output to File
    // -----------------------

    if (rank == 0)
    {

        int **outputarr = allocateMatrix(MatrixSize);    // allocate memory for output matrix

        writeResultToFile(rank, MatrixSize, outputarr, inputFile1, inputFile2);  // write output to file


        free(outputarr);
    }

    // -----------------------
    // Barrier Synchronization
    // -----------------------

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}
