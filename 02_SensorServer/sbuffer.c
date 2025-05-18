#include "sbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    sensor_data_t data;
    int processed;
    struct sbuffer_node *next;
} sbuffer_node_t;


/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int terminate;
};


sbuffer_t *sbuffer_init() {
    sbuffer_t *buffer = (sbuffer_t *)malloc(sizeof(sbuffer_t));
    if (!buffer) return NULL;

    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->terminate = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->cond, NULL);

    return buffer;
}

int sbuffer_free(sbuffer_t *buffer) {
    if (!buffer) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);
    sbuffer_node_t *current = buffer->head;
    while (current) {
        sbuffer_node_t *next = current->next;
        free(current);
        current = next;
    }
    pthread_mutex_unlock(&buffer->mutex);

    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->cond);
    free(buffer);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data) {
    if (!buffer || !data) return SBUFFER_NO_DATA;

    sbuffer_node_t *new_node = (sbuffer_node_t *)malloc(sizeof(sbuffer_node_t));
    if (!new_node) return SBUFFER_FAILURE;

    new_node->data = *data;
    new_node->processed = 0;
    new_node->next = NULL;

    pthread_mutex_lock(&buffer->mutex);

    if (buffer->tail) {
        buffer->tail->next = new_node;
    } else {
        buffer->head = new_node;
    }
    buffer->tail = new_node;

    pthread_cond_signal(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    if (!buffer || !data) return SBUFFER_NO_DATA;

    pthread_mutex_lock(&buffer->mutex);

    while (!buffer->head && !buffer->terminate) {
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
    }

    sbuffer_node_t *current = buffer->head;
    sbuffer_node_t *prev = NULL;

    while (current) {
        if (current->processed) {
            *data = current->data;
            if (prev) {
                prev->next = current->next;
            } else {
                buffer->head = current->next;
            }
            if (!current->next) {
                buffer->tail = prev;
            }
            free(current);
            pthread_mutex_unlock(&buffer->mutex);
            return SBUFFER_SUCCESS;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&buffer->mutex);
    return SBUFFER_FAILURE;
}


int sbuffer_is_empty(sbuffer_t *buffer) {
    if (!buffer) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);
    int is_empty = (buffer->head == NULL);
    pthread_mutex_unlock(&buffer->mutex);

    return is_empty;
}

int sbuffer_read_unprocessed(sbuffer_t *buffer, sensor_data_t *data, int processed_flag) {
    if (!buffer || !data) return SBUFFER_NO_DATA;

    pthread_mutex_lock(&buffer->mutex);

    sbuffer_node_t *current = buffer->head;
    while (current) {
        if (current->processed == processed_flag) {
            *data = current->data;
            pthread_mutex_unlock(&buffer->mutex);
            return SBUFFER_SUCCESS;
        }
        current = current->next;
    }

    if (buffer->terminate && !buffer->head) {
        pthread_mutex_unlock(&buffer->mutex);
        return SBUFFER_FAILURE;
    }

    pthread_mutex_unlock(&buffer->mutex);
    return SBUFFER_FAILURE;
}


int sbuffer_mark_processed(sbuffer_t *buffer, const sensor_data_t *data) {
    if (!buffer || !data) return SBUFFER_NO_DATA;

    pthread_mutex_lock(&buffer->mutex);

    sbuffer_node_t *current = buffer->head;
    while (current) {
        if (current->data.id == data->id && current->data.ts == data->ts) {
            current->processed = 1;
            pthread_cond_broadcast(&buffer->cond);
            pthread_mutex_unlock(&buffer->mutex);
            return SBUFFER_SUCCESS;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&buffer->mutex);
    return SBUFFER_FAILURE;
}

void sbuffer_terminate(sbuffer_t *buffer) {
    pthread_mutex_lock(&buffer->mutex);
    buffer->terminate = 1;
    pthread_cond_broadcast(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);
}

int sbuffer_is_terminated(sbuffer_t *buffer) {
    pthread_mutex_lock(&buffer->mutex);
    int result = (buffer->terminate && !buffer->head);
    pthread_mutex_unlock(&buffer->mutex);
    return result;
}
