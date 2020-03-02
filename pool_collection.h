#ifndef POOL_COLLECTION_H
#define POOL_COLLECTION_H

#include <stdbool.h>

typedef struct pool_node {
    void *pool;
    struct pool_node *next;
    struct pool_node *previous;
} pool_node_t;

typedef struct pool_collection {
    pool_node_t *first;
    pool_node_t *last;
} pool_collection_t;

/**
 * Dodaje pulę do kolekcji i zwraca wskaźnik na nowy węzeł,
 * w którym się ona znajduje.
 */
pool_node_t *add_pool(pool_collection_t *pool_collection, void *pool);

/**
 * Usuwa węzeł, w którym znajduje się niszczona pula, z kolekcji.
 */
void delete_pool_node(pool_collection_t *pool_collection, pool_node_t *node);

#endif