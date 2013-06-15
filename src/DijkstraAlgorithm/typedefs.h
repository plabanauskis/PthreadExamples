/**
* Programa: Dijkstra algorithm
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

typedef struct node_t;

typedef struct path_t
{
    int dist;
    node_t *a;
    node_t *b;
} path_t;

typedef struct node_t
{
    bool in_graph;
    unsigned int node_num;
    int dist;
    size_t path_count;
    path_t **paths;
} node_t;

typedef struct graph_t
{
    size_t node_count;
    node_t *nodes;
    path_t ***paths;
} graph_t;
