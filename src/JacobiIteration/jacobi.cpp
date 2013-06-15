/**
* Program: Jacobi iteration
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

#define SWAP(a,b,t) (((t) = (a)), ((a) = (b)), ((b) = (t)))

// Global Variables
float** g_old_array;
float** g_new_array;

size_t num_threads;
size_t iterations;
int array_size_per_thread;
int array_size;

pthread_barrier_t jacobi_barrier;

void* ThreadMain(void*);

int main(int argc, char *argv[])
{
   printf("Starting 2D Jacobi iteration...\n");

    if (argc < 3)
    {
        fprintf(stderr, "Required arguments:\n \
                        iterations - number of iterations \n \
                        array_size_per_thread - single thread array size\n \
                        num_threads - number of worker threads.\n");
        return -1;
    }

   array_size_per_thread = atoi(argv[1]);
   iterations = atoi(argv[2]);
   num_threads = atoi(argv[3]);
   array_size = (int)(sqrt((double)num_threads) * array_size_per_thread);

   g_old_array = (float**) malloc((array_size+2) * sizeof(float*));
   g_new_array = (float**) malloc((array_size+2) * sizeof(float*));
   for (int i = 0; i < array_size+2; i++)
   {
      g_old_array[i] = (float*) malloc((array_size+2) * sizeof(float));
      g_new_array[i] = (float*) malloc((array_size+2) * sizeof(float));

      for (int j = 0; j < array_size+2; j++)
      {
         g_old_array[i][j] = 0.0;
         g_new_array[i][j] = 0.0;
      }
   }

   for (int i = 1; i < array_size+1; i++)
   {
      for (int j = 1; j < array_size+1; j++)
      {
         g_old_array[i][j] = (float)(i * j);
         g_new_array[i][j] = 0.0;
      }
   }

   pthread_barrier_init(&jacobi_barrier, NULL, (unsigned int)num_threads);

   pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

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

   printf("The end.\n");

   return 0;
}

void* ThreadMain(void* threadid)
{
   size_t tid = (size_t)threadid;

   int x_coord = tid % ((int) sqrt((double)num_threads));
   int y_coord = tid / ((int) sqrt((double)num_threads));

   int start_index_x = (x_coord * array_size_per_thread) + 1;
   int end_index_x = ((x_coord+1) * array_size_per_thread) + 1;
   int start_index_y = (y_coord * array_size_per_thread) + 1;
   int end_index_y = ((y_coord+1) * array_size_per_thread) + 1;

   for (int k = 0; k < iterations; k++)
   {
      for (int i = start_index_x; i < end_index_x; i++)
      {
         for (int j = start_index_y; j < end_index_y; j++)
         {
            g_new_array[i][j] = (g_old_array[i-1][j] + g_old_array[i+1][j] + g_old_array[i][j-1] + g_old_array[i][j+1]) / 4;
         }
      }

      pthread_barrier_wait(&jacobi_barrier);

      if (tid == 0)
      {
         float **temp;
         SWAP(g_old_array, g_new_array, temp);
      }

      pthread_barrier_wait(&jacobi_barrier);
   }

   return 0;
}
