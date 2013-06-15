/**
* Program: Sparse matrix-vector multiplication
**/

#pragma once

#include "typedefs.h"

matrix_t * CreateMatrix(size_t num_rows, size_t num_columns);
void DestroyMatrix(matrix_t *matrixP);
int SetItemValue(matrix_t *matrixP, matrix_item_t value, size_t row, size_t column);

sparse_matrix_t * CompressMatrix(matrix_t *matrixP);
void DestroySparseMatrix(sparse_matrix_t *matrixP);
void DestroySparseMatrixItems(sparse_matrix_item_t *matrixItemP);

vector_t * CreateVector(size_t num_items);
void DestroyVector(vector_t *vectorP);