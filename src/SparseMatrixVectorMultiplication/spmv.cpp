/**
* Program: Sparse matrix-vector multiplication
**/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "typedefs.h"
#include "matrix.h"

#define RAND_SEED 46540 // input matrix generation seed
#define MAX_U_SHORT 65535
#define NON_ZERO_ITEMS_THRESHOLD (RAND_MAX * 0.2) // approximately 20% of the matrix elements will be non-zero

sparse_matrix_t * GenerateSparseMatrix(size_t rows, size_t columns, int seed);
vector_t * GenerateVector(size_t size, int seed);

size_t rows, columns, num_threads;
int last_row = -1;
pthread_mutex_t mutex; // mutex, protecting against multiple threads taking same matrix row
sparse_matrix_t *matrixP;
vector_t *vectorP;
vector_t *resultVectorP;

void * ThreadMain(void *args);
sparse_matrix_row_t * GetNextRow(sparse_matrix_t *matrix);

int main(int argc, char *argv[])
{
    printf("Starting sparse matrix-vector multiplication...\n");

    if (argc < 4)
    {
        fprintf(stderr, "Required arguments:\n \
                        rows - number of the matrix rows\n \
                        columns - number of the matrix columns, vector items count\n \
                        num_threads - number of worker threads.");
        return -1;
    }

    rows = atoi(argv[1]);
    columns = atoi(argv[2]);
    num_threads = atoi(argv[3]);

    pthread_mutex_init(&mutex, NULL);

    matrixP = GenerateSparseMatrix(rows, columns, RAND_SEED);
    ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(matrixP, "matrixP");
    vectorP = GenerateVector(columns, RAND_SEED);
    ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(vectorP, "vectorP");

    resultVectorP = CreateVector(rows);
    ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(resultVectorP, "resultVectorP");

    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);

    for (int i = 0; i < num_threads; i++)
    {
        if (SUCCESS != pthread_create(threads+i, NULL, ThreadMain, NULL))
        {
            printf("Klaida kuriant gija.\n");
            return -1;
        }
    }

    for (size_t i = 0; i < num_threads; i++)
    {
        pthread_join(*(threads+i), NULL);
    }

    DestroyVector(resultVectorP);
    DestroyVector(vectorP);
    DestroySparseMatrix(matrixP);

    printf("The end.\n");

    return 0;
}

sparse_matrix_t * GenerateSparseMatrix(size_t rows, size_t columns, int seed)
{
    srand(seed);

    matrix_t * matrix = CreateMatrix(rows, columns);
    ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(matrix, "matrix");

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (rand() > NON_ZERO_ITEMS_THRESHOLD)
            {
                matrix->rows[i].items[j] = 0;
            }
            else
            {
                matrix->rows[i].items[j] = rand() % (MAX_U_SHORT + 1);
            }
        }
    }

    sparse_matrix_t *spMatrixP = CompressMatrix(matrix);

    DestroyMatrix(matrix);

    return spMatrixP;
}

vector_t * GenerateVector(size_t size, int seed)
{
    vector_t *vectorP = CreateVector(size);

    for (int i = 0; i < vectorP->num_items; i++)
    {
        vectorP->items[i] = rand() % (MAX_U_SHORT + 1);
    }

    return vectorP;
}

void * ThreadMain(void *args)
{
    sparse_matrix_row_t *row;
    while ((row = GetNextRow(matrixP)) != NULL)
    {
        sparse_matrix_item_t * item = row->items;
        ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(item, "item");

        do
        {
            // TODO: are items 0 at the beginning?
            resultVectorP->items[row->index] += item->value * vectorP->items[item->column];
            item = item->next_item;
        } while (item != NULL);
    }

    return NULL;
}

sparse_matrix_row_t * GetNextRow(sparse_matrix_t *matrix)
{
    ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(matrix, "matrix"); 

    sparse_matrix_row_t *row = NULL;

    if (SUCCESS == pthread_mutex_lock(&mutex))
    {
        last_row++;

        if (last_row >= matrix->num_rows)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        row = matrix->rows+last_row;

        pthread_mutex_unlock(&mutex);
    }

    return row;
}
