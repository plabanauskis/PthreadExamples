/**
* Programa: Dijkstra algorithm
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include "typedefs.h"

#define RAND_SEED 46540 // graph generation seed
#define INFINITY -1

graph_t *graph;
size_t num_threads;
size_t path_length;
node_t *initial_node;
node_t *target_node;
bool threads_initialized = false;

void Traverse(pthread_t *);
void * ThreadMain(void*);
node_t *GetNeighbour(node_t *, int);
graph_t * GenerateGraph(size_t);
void DestroyGraph(graph_t *);
void PrintGraph(graph_t *);
void InitializeStartEndNodes(graph_t *);

static bool shortest_path_found = false;
static pthread_cond_t traverser_wait_condition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t traverser_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t worker_wait_condition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t worker_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static int waiting = 0;

int main(int argc, char *argv[])
{
    printf("Staring Dijkstra algorithm...\n");

    if (argc < 3)
    {
        fprintf(stderr, "Required arguments:\n \
                        num_nodes - number of graph nodes.\n \
                        num_threads - number of worker threads.");
        return -1;
    }

    int num_nodes = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    graph = GenerateGraph(num_nodes);
    InitializeStartEndNodes(graph);

#ifdef DEBUG
    printf("Start: %d\n", initial_node->node_num);
    printf("End: %d\n", target_node->node_num);
    PrintGraph(graph);
#endif

    num_threads -= 1; // leave some work for the main thread
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

    Traverse(threads);

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(*(threads+i), NULL);
    }

    DestroyGraph(graph);
    pthread_cond_destroy(&traverser_wait_condition);
    pthread_mutex_destroy(&traverser_wait_mutex);
    pthread_cond_destroy(&worker_wait_condition);
    pthread_mutex_destroy(&worker_wait_mutex);

    printf("The end.\n");

    return 0;
}

void Traverse(pthread_t *threads)
{
    node_t *nearest_node = NULL;

    pthread_mutex_lock(&traverser_wait_mutex);
    int x = 0;
    while (initial_node != target_node)
    {

        for (int i = 0; i < graph->node_count; i++)
        {
            if (nearest_node == NULL && graph->nodes[i].in_graph &&
                graph->nodes[i].dist != INFINITY)
            {
                nearest_node = graph->nodes + i;
            }
            else if (graph->nodes[i].in_graph && graph->nodes[i].dist != INFINITY &&
                graph->nodes[i].dist < nearest_node->dist)
            {
                nearest_node = graph->nodes + i;
            }
        }

        initial_node = nearest_node;
        nearest_node = NULL;

        if (!threads_initialized)
        {
            threads_initialized = true;

            for (int i = 0; i < num_threads; i++)
            {
                if (0 != pthread_create(threads+i, NULL, ThreadMain, (void *)i))
                {
                    fprintf(stderr, "Error creating a thread: %i.\n", i);
                    exit(1);
                }
            }
        }
        else
        {
            pthread_cond_broadcast(&worker_wait_condition);
        }

        pthread_cond_wait(&traverser_wait_condition, &traverser_wait_mutex);
        pthread_mutex_lock(&worker_wait_mutex);
        pthread_mutex_unlock(&worker_wait_mutex);

        initial_node->in_graph = false;
    }

    pthread_mutex_unlock(&traverser_wait_mutex);
}

void* ThreadMain(void* threadid)
{
    size_t tid = (size_t)threadid;

    pthread_mutex_lock(&worker_wait_mutex);

    while (initial_node != target_node)
    {
        int work_number = tid;

        while (work_number < initial_node->path_count)
        {
            node_t *neighbour = GetNeighbour(initial_node, work_number);

            if (neighbour != NULL)
            {
                int dist_to_node = initial_node->dist + initial_node->paths[tid]->dist;

                if (neighbour->dist == INFINITY || neighbour->dist > dist_to_node)
                {
                    neighbour->dist = dist_to_node;
                }
            }

            work_number += num_threads;
        }

        waiting++;
        if (waiting == num_threads)
        {
            waiting = 0;
            pthread_mutex_lock(&traverser_wait_mutex);
            pthread_mutex_unlock(&traverser_wait_mutex);
            pthread_cond_signal(&traverser_wait_condition);
        }
        pthread_cond_wait(&worker_wait_condition, &worker_wait_mutex);
    }

    pthread_mutex_unlock(&worker_wait_mutex);
    pthread_mutex_lock(&traverser_wait_mutex);
    pthread_mutex_unlock(&traverser_wait_mutex);
    pthread_cond_signal(&traverser_wait_condition);
    return NULL;
}

node_t *GetNeighbour(node_t *node, int path_number)
{
    if (node->paths[path_number]->a == NULL || node->paths[path_number]->b == NULL)
    {
        return NULL;
    }

    if (node->paths[path_number]->a == node)
    {
        return node->paths[path_number]->b;
    }
    else if (node->paths[path_number]->b == node)
    {
        return node->paths[path_number]->a;
    }

    return NULL;
}

graph_t * GenerateGraph(size_t num_nodes)
{
    srand(RAND_SEED);

    node_t *nodes = (node_t *)calloc(num_nodes, sizeof(node_t));
    path_t ***paths = (path_t ***)calloc(num_nodes, sizeof(path_t **));

    for (int i = 0; i < num_nodes; i++)
    {
        nodes[i].node_num = i;
        nodes[i].in_graph = true;
        nodes[i].dist = INFINITY;
        nodes[i].paths = (path_t **)calloc(num_nodes - 1, sizeof(path_t *));
    }

    for (int i = 0; i < num_nodes - 1; i++)
    {
        size_t path_seq = nodes[i].path_count;

        for (int j = i + 1; j < num_nodes; j++)
        {
            bool has_connection = (rand() % 2 == 0);

            if (has_connection)
            {
                nodes[i].paths[path_seq] = (path_t *)calloc(1, sizeof(path_t));
                nodes[i].paths[path_seq]->dist = rand() % 100;
                nodes[i].paths[path_seq]->a = nodes + i;
                nodes[i].paths[path_seq]->b = nodes + j;
                nodes[i].path_count++;

                size_t to_path_seq = nodes[j].path_count;
                nodes[j].paths[to_path_seq] = nodes[i].paths[path_seq];
                nodes[j].path_count++;

                path_seq++;
            }
        }

        paths[i] = (path_t **)malloc(sizeof(path_t) * nodes[i].path_count);
        for (int j = 0; j < nodes[i].path_count; j++)
        {
            paths[i][j] = nodes[i].paths[j];
        }
    }

    graph_t *graph = (graph_t *)calloc(1, sizeof(graph_t));
    graph->node_count = num_nodes;
    graph->nodes = nodes;
    graph->paths = paths;

    return graph;
}

void DestroyGraph(graph_t *graph)
{
    // TODO: free paths.

    for (int i = 0; i < graph->node_count; i++)
    {
        node_t *node = graph->nodes + i;

        free(node->paths);
    }

    free(graph->nodes);
    free(graph->paths);

    free(graph);
}

void PrintGraph(graph_t *graph)
{
    for (int i = 0; i < graph->node_count; i++)
    {
        node_t *node = graph->nodes+i;
        for (int j = 0; j < node->path_count; j++)
        {
            path_t path = *node->paths[j];
            printf("%d -> %d = %d + %d\n", node->node_num, path.a == node ? path.b->node_num : path.a->node_num, node->dist, path.dist);
        }

        printf("\n");
    }
}

void InitializeStartEndNodes(graph_t *graph)
{
    initial_node = graph->nodes + (rand() % graph->node_count);
    initial_node->dist = 0;

    while (target_node == NULL || target_node == initial_node)
    {
        target_node = graph->nodes + (rand() % graph->node_count);
    }
}
