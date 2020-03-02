#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include "runnable.h"

typedef struct work_node {
    runnable_t work;
    struct work_node *next;
} work_node_t;

typedef struct work_queue {
    int number_of_elements;
    work_node_t *first;
    work_node_t *last;

} work_queue_t;

/**
 * Daje w wyniku wskaźnik do nowej kolejki zadań.
 */
work_queue_t *work_queue_init(void);

/**
 * Sprawdza, czy kolejka jest pusta.
 */
bool work_queue_empty(work_queue_t *queue);

/**
 * Dodaje zadanie do kolejki.
 */
bool work_queue_add(work_queue_t *queue, runnable_t work);

/**
 * Pobiera zadanie z kolejki.
 */
runnable_t work_queue_get(work_queue_t *queue);

/**
 * Niszczy kolejkę.
 */
void work_queue_destroy(work_queue_t *queue);

#endif
