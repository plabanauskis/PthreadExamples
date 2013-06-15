/**
* Program: Matrix multiplication
**/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#define RAND_SEED 46540 // input matrix generation seed
#define MAX_U_SHORT 65535 // maximum matrix cell value

int **matA; // input matrix A
int **matB; // input matrix B
int **matC; // result matrix C
size_t m, n, p; // matrix dimensions (m - first matrix rows,
                // n - first matrix columns, second matrix rows,
                // p - second matrix columns)
size_t num_threads; // number of working threads
bool rand_initialized; // randomization initialization flag
size_t work_units; // number of work units for individual thread
size_t auxiliary_units; // unassigned work units that will be shared among threads

void * ThreadMain(void *); // main matrix multiplication function
int** GenerateMatrix(size_t rows, size_t columns, unsigned int seed); // input matrix generator

int main(int argc, char *argv[])
{
    printf("Starting matrix multiplication...\n");

    if (argc < 5)
    {
        fprintf(stderr, "Required arguments:\n \
                        m - first matrix row count\n \
                        n - first matrix column count, second matrix row count\n \
                        p - second matrix column count\n \
                        num_threads - number of worker threads.");
        return -1;
    }

    m = atoi(argv[1]);
    n = atoi(argv[2]);
    p = atoi(argv[3]);
    num_threads = atoi(argv[4]);

    matA = GenerateMatrix(m, n, RAND_SEED);
    matB = GenerateMatrix(n, p, RAND_SEED);

    matC = (int**) malloc(m * sizeof(int*));
    for (unsigned int i = 0; i < m; i++)
    {
        matC[i] = (int*) malloc(p * sizeof(int));
        for (unsigned int j = 0; j < p; j++)
        {
            matC[i][j] = 0;
        }
    }

    size_t spawnedThreads = m * p >= num_threads ? num_threads : m*p;

    work_units = (unsigned int)m * (unsigned int)p / spawnedThreads;
    auxiliary_units = (m * p) % spawnedThreads;

    pthread_t * threads = (pthread_t *)malloc(sizeof(pthread_t) * spawnedThreads);

    for (size_t i = 0; i < spawnedThreads; i++)
    {
        if (0 != pthread_create(threads+i, NULL, ThreadMain, (void *)i))
        {
            fprintf(stderr, "Error creating a thread: %i.\n", i);
            return -1;
        }
    }

    for (unsigned int i = 0; i < spawnedThreads; i++)
    {
        pthread_join(*(threads+i), NULL);
    }

    printf("The end.\n");

    return 0;
}

void * ThreadMain(void *threadid)
{
    size_t tid = (size_t)threadid;

    size_t thread_work_units = work_units;
    size_t start_cell = work_units * tid;
    if (tid < auxiliary_units)
    {
        thread_work_units += 1;
    }

    start_cell += (tid < auxiliary_units) ? tid : auxiliary_units;

    size_t column = start_cell % p;
    size_t row = start_cell / p;

    for (size_t i = 0; i < thread_work_units; i++)
    {
        matC[row][column] = 0;

        for (size_t j = 0; j < p; j++)
        {
            matC[row][column] += matA[row][j] * matB[j][column];
        }

        column++;

        if (column >= p)
        {
            column = 0;
            row++;
        }
    }

    return 0;
}

int** GenerateMatrix(size_t rows, size_t columns, unsigned int seed)
{
    if (!rand_initialized)
    {
        srand(seed);
        rand_initialized = true;
    }

    // Initializing matrix elements
    int** matrix = (int**) malloc(m * sizeof(int*));
    for (unsigned int i = 0; i < rows; i++)
    {
        matrix[i] = (int*) malloc(n * sizeof(int));
        for (unsigned int j = 0; j < columns; j++)
        {
            matrix[i][j] = rand() % (MAX_U_SHORT + 1);
        }
    }

    return matrix;
}

