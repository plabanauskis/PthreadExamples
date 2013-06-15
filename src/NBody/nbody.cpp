/**
* Program: Body movement in space simulation
**/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "typedefs.h"

#define RAND_SEED 46540

#define MAX_X 1000
#define MIN_X 0
#define MAX_Y 1000 
#define MIN_Y 0
#define MAX_W 1000
#define MIN_W 100
#define MAX_V 50
#define MIN_V 0
#define DELTA_T 1

const double G = 6.67259; // Gravity constant

size_t num_bodies, num_threads, iterations;
Body *current_bodies;
Body *new_bodies;

// Synchronization variables
pthread_mutex_t barrier_init_mutex;
pthread_barrier_t calc_iter_barrier;
bool calc_iter_barrier_initialized = false;
pthread_barrier_t update_iter_barrier;
bool update_iter_barrier_initialized = false;

void * ThreadMain(void *args);
void CalculateNewInfo(size_t start, size_t end);
void UpdateNewInfo(size_t start, size_t end);

int main(int argc, char *argv[])
{
    printf("Starting body movement in space simulation...\n");

    if (argc < 4)
    {
        fprintf(stderr, "Required arguments:\n \
                        num_bodies - number of bodies\n \
                        iterations - number of iterations\n \
                        num_threads - number of worker threads.");
        return -1;
    }

    num_bodies = (size_t)atoi(argv[1]);
    iterations = (size_t)atoi(argv[2]);
    num_threads = (size_t)atoi(argv[3]);

    current_bodies = (Body *)calloc(num_bodies, sizeof(Body));
    new_bodies = (Body *)calloc(num_bodies, sizeof(Body));

    srand(RAND_SEED);

    for (int i = 0; i < num_bodies; i++)
    {
        current_bodies[i].x = rand() % (MAX_X - MIN_X) + MIN_X;
        current_bodies[i].y = rand() % (MAX_Y - MIN_Y) + MIN_Y;
        current_bodies[i].vx = rand() % (MAX_V - MIN_V) - (MAX_V + MIN_V) / 2;
        current_bodies[i].vy = rand() % (MAX_V - MIN_V) - (MAX_V + MIN_V) / 2;
        current_bodies[i].w = rand() % (MAX_W - MIN_W) + MIN_W;
    }

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

    pthread_mutex_init(&barrier_init_mutex, NULL);

    for (int i = 0; i < num_threads; i++)
    {
        if (0 != pthread_create(threads+i, NULL, ThreadMain, (void *)i))
        {
            fprintf(stderr, "Error creating a thread: %i.\n", i);
            return -1;
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(*(threads+i), NULL);
    }

    pthread_mutex_destroy(&barrier_init_mutex);

    printf("The end.\n");

    return 0;
}

void * ThreadMain(void *args)
{
    size_t tid = (size_t)args;

    size_t remainder = num_bodies % num_threads;
    size_t parts = num_bodies / num_threads;
    size_t start = tid * parts + (tid >= remainder ? remainder : tid);
    size_t end = start + parts + (tid >= remainder ? 0 : 1);

    for (int i = 0; i < iterations; i++)
    {
        pthread_mutex_lock(&barrier_init_mutex);
        if (!calc_iter_barrier_initialized)
        {
            if (update_iter_barrier_initialized)
            {
                pthread_barrier_destroy(&update_iter_barrier);
            }

            pthread_barrier_init(&calc_iter_barrier, NULL, (unsigned int)num_threads);
            calc_iter_barrier_initialized = true;
            update_iter_barrier_initialized = false;
        }
        pthread_mutex_unlock(&barrier_init_mutex);

        CalculateNewInfo(start, end);

        pthread_barrier_wait(&calc_iter_barrier);

        pthread_mutex_lock(&barrier_init_mutex);
        if (!update_iter_barrier_initialized)
        {
            if (calc_iter_barrier_initialized)
            {
                pthread_barrier_destroy(&calc_iter_barrier);
            }

            pthread_barrier_init(&update_iter_barrier, NULL, (unsigned int)num_threads);
            update_iter_barrier_initialized = true;
            calc_iter_barrier_initialized = false;
        }
        pthread_mutex_unlock(&barrier_init_mutex);

        UpdateNewInfo(start, end);

        pthread_barrier_wait(&update_iter_barrier);
    }

    return NULL;
}

void CalculateNewInfo(size_t start, size_t end)
{
    for(size_t i = start; i < end; i++)
    {
        Body *current_body = current_bodies+i;
        Body *new_body = new_bodies+i;

        new_body->w = current_body->w;
        new_body->vx = current_body->vx;
        new_body->vy = current_body->vy;
        new_body->x = current_body->x;
        new_body->y = current_body->y;

        for (int j = 0; j < num_bodies; j++)
        {
            if (j == i)
            {
                continue;
            }

            Body *current_other_body = current_bodies+j;

            double delta_x = new_body->x - current_other_body->x;
            double delta_y = new_body->y - current_other_body->y;
            double distance = sqrt(pow(delta_x,2) + pow(delta_y,2));

            if (distance == 0)
            {
                continue;
            }

            double force = G * current_other_body->w / (distance * distance);
            new_body->vx = new_body->vx + DELTA_T * force * delta_x / distance;
            new_body->vy = new_body->vy + DELTA_T * force * delta_y / distance;
            new_body->x = new_body->x + new_body->vx * DELTA_T;
            new_body->y = new_body->y + new_body->vy * DELTA_T;
        }
    }
}

void UpdateNewInfo(size_t start, size_t end)
{
    for(size_t i = start; i < end; i++)
    {
        Body *current_body = current_bodies+i;
        Body *new_body = new_bodies+i;

        current_body->w = new_body->w;
        current_body->vx = new_body->vx;
        current_body->vy = new_body->vx;
        current_body->x = new_body->x;
        current_body->y = new_body->y;
    }
}

