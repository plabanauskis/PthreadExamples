/**
* Program: Body movement in space simulation
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

typedef struct Body
{
    double x, y;   // position
    double vx, vy; // velocity
    double w;      // mass
} Body;

