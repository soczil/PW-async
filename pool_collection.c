#include <stdlib.h>
#include "pool_collection.h"

pool_node_t *add_pool(pool_collection_t *pool_collection, void *pool) {
    if (pool_collection == NULL) {
        return NULL;
    }

    pool_node_t *new_node = (pool_node_t *) malloc(sizeof(pool_node_t));
    if (new_node == NULL) {
        return NULL;
    }

    new_node->pool = pool;
    new_node->next = NULL;
    new_node->previous = NULL;

    if (pool_collection->last != NULL) {
        new_node->previous = pool_collection->last;
        pool_collection->last->next = new_node;
    } else {
        pool_collection->first = new_node;
    }
    pool_collection->last = new_node;

    return new_node;
}

void delete_pool_node(pool_collection_t *pool_collection, pool_node_t *node) {
    if (node != NULL && pool_collection != NULL) {
        if (node == pool_collection->first && node == pool_collection->last) {
            pool_collection->first = NULL;
            pool_collection->last = NULL;
        } else if (node == pool_collection->first) {
            pool_collection->first = node->next;
            node->next->previous = NULL;
        } else if (node == pool_collection->last) {
            pool_collection->last = node->previous;
            node->previous->next = NULL;
        } else {
            node->previous->next = node->next;
            node->next->previous = node->previous;
        }
    }

    free(node);
}
