#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"
#include "err.h"

#define MULTIPLIER 1000
#define THREADS 4

int *sums;
pthread_mutex_t *mutexes;

/**
 * Struktura zawierająca wszystkie potrzebne informacje o komórce macierzy.
 */
typedef struct cell_info {
    int value;
    int milis;
    int line_number;
} cell;

static int count_index(int i, int j, int n) {
    return ((i * n) + j);
}

/**
 * Funkcja usypia na podaną liczbę milisekund, a następnie dodaje wartość
 * komórki do sumy wiersza.
 */
static void count_cell(void *arg, size_t argsz __attribute__((unused))) {
    int err;
    cell *c = (cell *) arg;
    
    usleep(c->milis * MULTIPLIER);

    if ((err = pthread_mutex_lock(&(mutexes[c->line_number]))) != 0) {
        syserr(err, "mutex lock failed");
    }

    sums[c->line_number] += c->value;

    if ((err = pthread_mutex_unlock(&(mutexes[c->line_number]))) != 0) {
        syserr(err, "mutex unlock failed");
    }
}

int main(void) {
    cell *matrix = NULL;
    int err, k, n, value, milis, index;
    thread_pool_t pool;
    thread_pool_init(&pool, THREADS);

    scanf("%d\n%d", &k, &n);

    matrix = (cell *) malloc(k * n * sizeof(cell));
    if (matrix == NULL) {
        exit(1);
    }

    sums = (int *) malloc(k * sizeof(int));
    if (sums == NULL) {
        exit(1);
    }

    mutexes = (pthread_mutex_t *) malloc(k * sizeof(pthread_mutex_t));
    if (mutexes == NULL) {
        exit(1);
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            index = count_index(i, j, n);
            scanf("%d %d", &value, &milis);
            matrix[index].value = value;
            matrix[index].milis = milis;
            matrix[index].line_number = i;
        }
        
        if ((err = pthread_mutex_init(&(mutexes[i]), 0)) != 0) {
            syserr(err, "mutex init failed");
        }

        sums[i] = 0;
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            index = count_index(i, j, n);
            defer(&pool, (runnable_t){.function = count_cell,
                                      .arg = &(matrix[index]),
                                      .argsz = sizeof(cell *)});
        }
    }

    thread_pool_destroy(&pool);

    // wypisywanie dopiero po zniszczeniu puli, aby wszystkie
    // sumy zdążyły się policzyć
    for (int i = 0; i < k; i++) {
        printf("%d\n", sums[i]);
    }

    free(matrix);
    free(sums);
    free(mutexes);

    return 0;
}
