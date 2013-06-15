/**
* Program: Sparse matrix-vector multiplication
**/

#include "matrix.h"
#include <stdlib.h>
#include <string.h>

#define ASSERT_PTR_OR_RETURN_NULL(ptr) \
    if (ptr == NULL) \
        return NULL;

#define ASSERT_PTR_OR_RETURN_FAILURE(ptr) \
    if (ptr == NULL) \
        return FAILURE;

#define ASSERT_PTR_OR_RETURN(ptr) \
    if (ptr == NULL) \
        return;

matrix_t * CreateMatrix(size_t num_rows, size_t num_columns)
{
    matrix_row_t *rows = (matrix_row_t *)calloc(num_rows, sizeof(matrix_row_t));
    ASSERT_PTR_OR_RETURN_NULL(rows);

    for (int i = 0; i < num_rows; i++)
    {
        rows[i].items = (matrix_item_t *)calloc(num_columns, sizeof(matrix_item_t));
        ASSERT_PTR_OR_RETURN_NULL(rows[i].items);
    }

    matrix_t *matrixP = (matrix_t *)calloc(1, sizeof(matrix_t));
    ASSERT_PTR_OR_RETURN_NULL(matrixP);

    matrixP->num_rows = num_rows;
    matrixP->num_columns = num_columns;
    matrixP->rows = rows;

    return matrixP;
}

void DestroyMatrix(matrix_t *matrixP)
{
    if (matrixP == NULL)
    {
        return;
    }

    for (int i = 0; i < matrixP->num_rows; i++)
    {
        free(matrixP->rows[i].items);
    }

    free(matrixP->rows);
    free(matrixP);
}

int SetItemValue(matrix_t *matrixP, matrix_item_t value, size_t row, size_t column)
{
    ASSERT_PTR_OR_RETURN_FAILURE(matrixP);

    if (row >= matrixP->num_rows || column >= matrixP->num_columns)
    {
        return FAILURE;
    }

    matrixP->rows[row].items[column] = value;
}

sparse_matrix_t * CompressMatrix(matrix_t *matrixP)
{
    ASSERT_PTR_OR_RETURN_NULL(matrixP);

    sparse_matrix_t *spMatrixP = (sparse_matrix_t *)calloc(1, sizeof(sparse_matrix_t));
    ASSERT_PTR_OR_RETURN_NULL(spMatrixP);

    spMatrixP->num_rows = matrixP->num_rows;
    spMatrixP->rows = (sparse_matrix_row_t *)calloc(matrixP->num_rows, sizeof(sparse_matrix_row_t));
    ASSERT_PTR_OR_RETURN_NULL(spMatrixP->rows);

    for (int i = 0; i < spMatrixP->num_rows; i++)
    {
        sparse_matrix_item_t *item = NULL;

        matrix_row_t *origRowP = matrixP->rows+i;

        for (int j = 0; j < matrixP->num_columns; j++)
        {
            matrix_item_t origItem = *(origRowP->items+j);

            if (origItem != 0)
            {
                sparse_matrix_item_t * new_item = (sparse_matrix_item_t *)calloc(1, sizeof(sparse_matrix_item_t));
                ASSERT_PTR_OR_RETURN_NULL(new_item);
                if (item == NULL)
                {
                    item = new_item;
                }
                else
                {
                    item->next_item = new_item;
                    item = item->next_item;
                }

                item->column = j;
                item->value = origItem;
                item->next_item = NULL;

                spMatrixP->rows[i].index = i;
                if (spMatrixP->rows[i].items == NULL)
                {
                    spMatrixP->rows[i].items = item;
                }
            }
        }
    }

    return spMatrixP;
}

void DestroySparseMatrix(sparse_matrix_t *matrixP)
{
    for (int i = 0; i < matrixP->num_rows; i++)
    {
        DestroySparseMatrixItems(matrixP->rows[i].items);
    }

    free(matrixP->rows);
    free(matrixP);
}

void DestroySparseMatrixItems(sparse_matrix_item_t *matrixItemP)
{
    ASSERT_PTR_OR_RETURN(matrixItemP);

    if (matrixItemP->next_item != NULL)
    {
        DestroySparseMatrixItems(matrixItemP->next_item);
    }

    free(matrixItemP);
}

vector_t * CreateVector(size_t num_items)
{
    vector_item_t *items = (vector_item_t *)calloc(num_items, sizeof(vector_item_t));
    ASSERT_PTR_OR_RETURN_NULL(items);

    vector_t *vectorP = (vector_t *)calloc(1, sizeof(vector_t));
    ASSERT_PTR_OR_RETURN_NULL(vectorP);
    vectorP->num_items = num_items;
    vectorP->items = items;

    return vectorP;
}

void DestroyVector(vector_t *vectorP)
{
    ASSERT_PTR_OR_RETURN(vectorP);

    free(vectorP->items);
    free(vectorP);
}
