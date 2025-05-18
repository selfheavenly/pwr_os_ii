#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {
    if (list == NULL || *list == NULL) return;

    for (dplist_node_t *curr = (*list)->head, *next; curr != NULL; curr = next) {
        next = curr->next;
        if (free_element && (*list)->element_free != NULL) {
            (*list)->element_free(&(curr->element));
        }
        free(curr);
    }
    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    if (list == NULL) return NULL;

    dplist_node_t *new_node = malloc(sizeof(dplist_node_t));
    new_node->element = insert_copy && list->element_copy ? list->element_copy(element) : element;
    new_node->prev = new_node->next = NULL;

    if (list->head == NULL || index <= 0) {
        new_node->next = list->head;
        if (list->head != NULL) list->head->prev = new_node;
        list->head = new_node;
        return list;
    }

    dplist_node_t *curr = list->head;
    for (int i = 0; curr->next != NULL && i < index - 1; i++) {
        curr = curr->next;
    }

    new_node->next = curr->next;
    if (curr->next != NULL) curr->next->prev = new_node;
    curr->next = new_node;
    new_node->prev = curr;

    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if (list == NULL || list->head == NULL) return list;

    dplist_node_t *curr = list->head;
    if (index <= 0) {
        list->head = curr->next;
        if (list->head != NULL) list->head->prev = NULL;
    } else {
        for (int i = 0; curr->next != NULL && i < index; i++) {
            curr = curr->next;
        }
        if (curr->prev != NULL) curr->prev->next = curr->next;
        if (curr->next != NULL) curr->next->prev = curr->prev;
    }

    if (free_element && list->element_free != NULL) {
        list->element_free(&(curr->element));
    }
    free(curr);

    return list;
}

int dpl_size(dplist_t *list) {
    if (list == NULL) return -1;

    int size = 0;
    dplist_node_t *curr = list->head;

    for (; curr != NULL; size++) {
        curr = curr->next;
    }

    return size;
}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;

    dplist_node_t *curr = list->head;
    for (int i = 0; curr->next != NULL && i < index; i++) {
        curr = curr->next;
    }

    return curr->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    if (list == NULL) return -1;

    dplist_node_t *curr = list->head;

    for (int i = 0; curr != NULL; i++) {
        if (list->element_compare(curr->element, element) == 0) {
            return i;
        }
        curr = curr->next;
    }
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;

    dplist_node_t *curr = list->head;
    for (int i = 0; curr->next != NULL && i < index; i++) {
        curr = curr->next;
    }
    return curr;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || list->head == NULL || reference == NULL) return NULL;

    dplist_node_t *curr = list->head;
    while (curr != NULL) {
        if (curr == reference) return curr->element;
        curr = curr->next;
    }
    return NULL;
}
