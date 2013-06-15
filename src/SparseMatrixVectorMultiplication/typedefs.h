/**
* Program: Sparse matrix-vector multiplication
**/

#pragma once

#include <stdio.h>

#define ASSERT_PTR_OR_RETURN_EXIT_WITH_ERROR(ptr, var_name) \
    if (ptr == NULL) \
        { \
        printf("Assertion error: %s is NULL.\n", var_name);\
        exit(1); \
        }

#define SUCCESS 0
#define FAILURE 1

typedef unsigned int matrix_item_t;

typedef struct matrix_row_t
{
    matrix_item_t *items;
} matrix_row_t;

typedef struct matrix_t
{
    size_t num_rows;
    size_t num_columns;
    matrix_row_t *rows;
} matrix_t;

typedef matrix_item_t vector_item_t;

typedef struct vector_t
{
    size_t num_items;
    vector_item_t *items;
} vector_t;

typedef struct sparse_matrix_item_t
{
    size_t column;
    matrix_item_t value;
    sparse_matrix_item_t *next_item;
} sparse_matrix_item_t;

typedef struct sparse_matrix_row_t
{
    size_t index;
    sparse_matrix_item_t *items;
} sparse_matrix_row_t;

typedef struct sparse_matrix_t
{
    size_t num_rows;
    sparse_matrix_row_t *rows;
} sparse_matrix_t;
