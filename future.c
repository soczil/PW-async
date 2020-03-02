#include <stdlib.h>
#include "future.h"
#include "err.h"

typedef void *(*function_t)(void *);

static void count_async(void *arg, size_t argsz __attribute__((unused))) {
    int err;
    future_t *future = (future_t *) arg;
    callable_t callable = future->callable;

    future->result = callable.function(callable.arg,
                                       callable.argsz, future->result_size);

    if ((err = pthread_mutex_unlock(&(future->mutex))) != 0) {
        syserr(err, "mutex unlock failed");
    }
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    int err;

    future->result_size = NULL;
    future->result = NULL;
    future->callable = callable;
    future->map_function = NULL;

    if ((err = pthread_mutex_init(&(future->mutex), 0)) != 0) {
        syserr(err, "mutex init failed");
    }

    if ((err = pthread_mutex_lock(&(future->mutex))) != 0) {
        syserr(err, "mutex lock failed");
    }

    if ((err = defer(pool, (runnable_t){.function = count_async,
                                        .arg = future,
                                        .argsz = sizeof(future_t *)})) != 0) {
        return -1;
    }

    return 0;
}

static void *count_map(void *arg, size_t argsz __attribute__((unused)), size_t *result_size) {
    future_t *future = (future_t *) arg;
    void *result = await(future);

    return future->map_function(result, sizeof(result), result_size);
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    from->map_function = function;

    if (async(pool, future, (callable_t){.function = count_map,
                                         .arg = from,
                                         .argsz = sizeof(future_t *)}) != 0) {
        return -1;
    }

    return 0;
}

void *await(future_t *future) {
    int err;
    void *result = NULL;

    if ((err = pthread_mutex_lock(&(future->mutex))) != 0) {
        syserr(err, "mutex lock failed");
    }

    result = future->result;

    if ((err = pthread_mutex_unlock(&(future->mutex))) != 0) {
        syserr(err, "mutex unlock failed");
    }

    return result;
}
