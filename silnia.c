#include <stdio.h>
#include <stdlib.h>
#include "threadpool.h"
#include "future.h"
#include "err.h"

#define THREADS 3

/**
 * Para: liczba i jej silnia.
 */
typedef struct pair_of_two_elements {
    unsigned long long factorial;
    int number;
} pair;

/**
 * Oblicza silnię liczby o jeden większej od liczby przekazanej
 * w strukturze pair.
 */
static void *count(void *arg, size_t argsz __attribute__((unused)),
            size_t *result_size __attribute__((unused))) {
    pair *p = (pair *) arg;
    
    p->number++;
    p->factorial = p->factorial * (unsigned long long) p->number;

    return p;
}

/**
 * Niszczy mutexy z utworzonych strukur future_t.
 */
static void destroy_future_mutexes(future_t *futures, int n) {
    int err;

    for (int i = 0; i < n; i++) {
        if ((err = pthread_mutex_destroy(&(futures[i].mutex))) != 0) {
            syserr(err, "mutex destroy failed");
        }
    }
}

int main(void) {
    int n;
    future_t *futures = NULL;
    pair *p = NULL, *result = NULL;
    thread_pool_t pool;
    thread_pool_init(&pool, THREADS);

    scanf("%d", &n);
    
    // można było rozpatrzeć jeden przypadek gdy n == 0 i dać w wyniku 1,
    // ale chciałem, żeby wszystko obliczało się przy pomocy mechanizmu future
    if (n == 0) {
        futures = (future_t *) malloc(sizeof(future_t));
    } else {
        futures = (future_t *) malloc(n * sizeof(future_t));
    }

    if (futures == NULL) {
        exit(1);
    }

    p = (pair *) malloc(sizeof(pair));
    if (p == NULL) {
        exit(1);
    }

    p->factorial = 1;
    p->number = 0;

    if(async(&pool, &(futures[0]), (callable_t){.function = count,
                                                .arg = p,
                                                .argsz = sizeof(pair *)}) != 0) {
        exit(1);
    }
    
    for (int i = 1; i < n; i++) {
        map(&pool, &(futures[i]), &(futures[i - 1]), count);
    }

    if (n == 0) {
        result = await(&(futures[0]));
    } else {
        result = await(&(futures[n - 1]));
    }

    printf("%llu\n", result->factorial);

    destroy_future_mutexes(futures, n);
    thread_pool_destroy(&pool);
    free(p);
    free(futures);

    return 0;
}
