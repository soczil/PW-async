#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>
#include "runnable.h"
#include "work_queue.h"
#include "pool_collection.h"

typedef struct thread_pool {
    size_t number_of_threads;
    pthread_mutex_t mutex;
    pthread_cond_t for_work;
    pthread_attr_t attr;
    void *retval;
    work_queue_t *queue; // wskaźnik na kolejkę przechowująca zadania
    bool end_pool;
    pthread_t *threads_arr;
    pool_node_t *pool_node; // wskaźnik na węzeł wkładany do pool_collection
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif
