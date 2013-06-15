/**
* Program: Calculating PI
**/

#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <random>
#endif

#define RAND_SEED 46540 // input matrix generation seed
#define MAX_THREADS 1024

void * ThreadMain(void *);
double area_value(double over, double under);

pthread_mutex_t add;

#ifndef _MSC_VER
drand48_data status[MAX_THREADS];
drand48_data *pts[MAX_THREADS];
#endif

size_t iterations;
size_t num_threads;
double total_over = 0, total_under = 0;

int main(int argc, char *argv[])
{
    printf("Starting PI calculation...\n");

    if (argc < 3)
    {
        fprintf(stderr, "Required arguments:\n \
                        iterations - number of iterations\n \
                        num_threads - number of worker threads.");
        return -1;
    }

    iterations = (size_t)atoi(argv[1]);
    num_threads = (size_t)atoi(argv[2]);

    iterations = iterations / num_threads;

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

    pthread_mutex_init(&add,NULL);

    for (int i=0; i < num_threads; i++)
    {
        if (0 != pthread_create(threads+i, NULL, ThreadMain, (void *)i))
        {
            fprintf(stderr, "Error creating a thread: %i.\n", i);
            return -1;
        }
    }

    for (int i=0; i < num_threads; i++)
    {
        pthread_join(*(threads+i), NULL);
    }

    printf("The end.\n");

    return 0;
}
void * ThreadMain(void *t)
{
    size_t tid = (size_t) t;
    double over = 0, under = 0;
    double x, y, *ptx, *pty;

#ifndef _MSC_VER
    pts[tid] = &status[tid];
    srand48_r(RAND_SEED * tid, pts[tid]); // creating a local random number generator
#else
    srand(RAND_SEED);
#endif

    ptx = &x; pty = &y;
    double count = 0;

    for (double n = tid; n < iterations; n += num_threads)
    {
        count++;
#ifndef _MSC_VER
        drand48_r(pts[tid], ptx);
        drand48_r(pts[tid], pty);
#else
        *ptx = rand();
        *pty = rand();
#endif

        if (y > sqrt(1. - x*x)) over++;
        else under++;
    }

    pthread_mutex_lock(&add);

    total_over += over;
    total_under += under;

    pthread_mutex_unlock(&add);

    return NULL;
}

double area_value(double over, double under)
{
    double value;
    value = under / (over + under);
    return value;
}

