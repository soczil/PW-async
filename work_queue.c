#include <stdlib.h>
#include <stdbool.h>
#include "work_queue.h"

work_queue_t *work_queue_init(void) {
    work_queue_t *queue = NULL;

    queue = (work_queue_t *) malloc(sizeof(work_queue_t));
    if (queue == NULL) {
        return NULL;
    } else {
        queue->number_of_elements = 0;
        queue->first = NULL;
        queue->last = NULL;
        return queue;
    }
}

bool work_queue_empty(work_queue_t *queue) {
    if (queue == NULL) {
        return true;
    } else {
        return (queue->number_of_elements == 0);
    }
}

bool work_queue_add(work_queue_t *queue, runnable_t work) {
    work_node_t *node = NULL;

    if (queue == NULL) {
        return false;
    }

    node = (work_node_t *) malloc(sizeof(work_node_t));
    if (node == NULL) {
        return false;
    } else {
        node->work = work;
        node->next = NULL;
        if (queue->last != NULL) {
            queue->last->next = node;
        } else {
            queue->first = node;
        }
        queue->last = node;
        queue->number_of_elements++;
        return true;
    }
}

runnable_t work_queue_get(work_queue_t *queue) {
    work_node_t *node = NULL;
    runnable_t work;

    if (queue != NULL) {
        node = queue->first;
        if (node != NULL) {
            queue->first = node->next;
            if (queue->first == NULL) {
                queue->last = NULL;
            }
            work = node->work;
            free(node);
            queue->number_of_elements--;
        }
    }

    return work;
}

void work_queue_destroy(work_queue_t *queue) {
    work_node_t *node = NULL, *to_destroy = NULL;

    if (queue != NULL) {
        node = queue->first;
        while (node != NULL) {
            to_destroy = node;
            node = node->next;
            free(to_destroy);
        }

        free(queue);
    }
}
