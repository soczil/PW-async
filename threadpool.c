#include <stdlib.h>
#include <signal.h>
#include "threadpool.h"
#include "err.h"

// kolekcja przetrzymująca wszystkie zainicjowane pule
pool_collection_t pool_collection;

bool need_for_sigint_change = true;

static void change_sigint(int signal) {
    if (signal == SIGINT) {
        pool_node_t *node = pool_collection.first;

        while (node != NULL) {
            thread_pool_destroy(node->pool);
        }
    }
}

static void init_sigint_behaviour() {
    struct sigaction action;
    sigset_t block_mask;

    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);

    action.sa_flags = 0;
    action.sa_handler = change_sigint;
    action.sa_mask = block_mask;

    if (sigaction(SIGINT, &action, 0) == -1) {
        syserr(-1, "sigaction failed");
    }
}

static void *run(void *arg) {
    int err;
    thread_pool_t *pool = (thread_pool_t *) arg;
    runnable_t work;

    if (pool != NULL) {
        while (true) {
            if ((err = pthread_mutex_lock(&(pool->mutex))) != 0) {
                syserr(err, "mutex lock failed");
            }

            while (!pool->end_pool && work_queue_empty(pool->queue)) {
                if ((err = pthread_cond_wait(&(pool->for_work),
                                             &(pool->mutex))) != 0) {
                    syserr(err, "cond wait failed");
                }
            }

            // wychodzę, tylko jeśli pula ma się zakończyć oraz
            // nie pozostały żadne zadania do wykonania
            if (pool->end_pool && work_queue_empty(pool->queue)) {
                break;
            }

            work = work_queue_get(pool->queue);

            if ((err = pthread_mutex_unlock(&(pool->mutex))) != 0) {
                syserr(err, "mutex unlock failed");
            }

            work.function(work.arg, work.argsz);
        }
    }

    if ((err = pthread_mutex_unlock(&(pool->mutex)))) {
        syserr(err, "mutex unlock failed");
    }

    return NULL;
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
    int err;
    pool_node_t *new_node = NULL;

    if (pool == NULL) {
        return -1;
    }

    if (need_for_sigint_change) {
        init_sigint_behaviour();
        need_for_sigint_change = false;
    }

    pool->number_of_threads = num_threads;
    pool->end_pool = false;
    pool->threads_arr = NULL;
    pool->pool_node = NULL;
    pool->queue = work_queue_init(); // tworzenie kolejki zadań

    if (pool->queue == NULL) {
        return -1;
    }

    if ((err = pthread_mutex_init(&(pool->mutex), 0)) != 0) {
        syserr(err, "mutex init failed");
    }

    if ((err = pthread_cond_init(&(pool->for_work), 0)) != 0) {
        syserr(err, "cond init failed");
    }

    if ((err = pthread_attr_init(&(pool->attr))) != 0) {
        syserr(err, "attr init failed");
    }

    if ((err = pthread_attr_setdetachstate(&(pool->attr),
                                           PTHREAD_CREATE_JOINABLE)) != 0) {
        syserr(err, "attr setdetachstate failed");
    }

    pool->threads_arr = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
    if (pool->threads_arr == NULL) {
        return -1;
    }

    for (size_t i = 0; i < num_threads; i++) {
        if ((err = pthread_create(&(pool->threads_arr[i]),
                                  &(pool->attr), run, pool)) != 0) {
            syserr(err, "thread create failed");
        }
    }

    // dodawanie nowej puli do kolekcji
    new_node = add_pool(&pool_collection, pool);
    if (new_node == NULL) {
        return -1;
    }

    pool->pool_node = new_node;

    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {
    int err;

    if ((err = pthread_mutex_lock(&(pool->mutex))) != 0) {
        syserr(err, "mutex lock failed");
    }

    pool->end_pool = true;

    if ((err = pthread_mutex_unlock(&(pool->mutex)))) {
        syserr(err, "mutex unlock failed");
    }

    if ((err = pthread_cond_broadcast(&(pool->for_work))) != 0) {
        syserr(err, "cond broadcast failed");
    }

    for (size_t i = 0; i < pool->number_of_threads; i++) {
        if ((err = pthread_join(pool->threads_arr[i], &(pool->retval))) != 0) {
            syserr(err, "thread join failed");
        }
    }

    // usuwanie kolejki zadań
    work_queue_destroy(pool->queue);
    free(pool->threads_arr);

    if ((err = pthread_cond_destroy(&(pool->for_work))) != 0) {
        syserr(err, "cond destroy failed");
    }

    if ((err = pthread_mutex_destroy(&(pool->mutex))) != 0) {
        syserr(err, "mutex destroy failed");
    }

    if ((err = pthread_attr_destroy(&(pool->attr))) != 0) {
        syserr(err, "attr destroy failed");
    }

    // usuwanie puli z kolekcji
    delete_pool_node(&pool_collection, pool->pool_node);
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    int err;

    if ((err = pthread_mutex_lock(&(pool->mutex))) != 0) {
        syserr(err, "mutex lock failed");
    }

    if (!pool->end_pool) {
        if ((err = pthread_mutex_unlock(&(pool->mutex)))) {
            syserr(err, "mutex unlock failed");
        }

        if (!work_queue_add(pool->queue, runnable)) {
            return -1;
        }

        if ((err = pthread_cond_signal(&(pool->for_work))) != 0) {
            syserr(err, "cond signal failed");
        }

        return 0;
    } else {
        if ((err = pthread_mutex_unlock(&(pool->mutex)))) {
            syserr(err, "mutex unlock failed");
        }

        return -1;
    }
}
