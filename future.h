#ifndef FUTURE_H
#define FUTURE_H

#include "threadpool.h"

typedef struct callable {
    void *(*function)(void *, size_t, size_t *);
    void *arg;
    size_t argsz;
} callable_t;

typedef struct future {
    void *result;
    pthread_mutex_t mutex;
    callable_t callable;
    size_t *result_size;
    // pomocnicza funkcja używana w mapie
    void *(*map_function)(void *, size_t, size_t *);
} future_t;

/**
 * Zleca wywołanie na puli pool funkcji z argumentu callable.
 */
int async(thread_pool_t *pool, future_t *future, callable_t callable);

/**
 * Zleca jakiejś puli wywołanie funkcji na wyniku from.
 */
int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *));

/**
 * Czeka na zakończenie wywołania funkcji z argumentu callable
 * w async i zwraca jej wynik.
 */
void *await(future_t *future);

#endif
