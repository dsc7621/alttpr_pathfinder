/**
 * deque.c
 * -------
 * A deque implementation using a circular buffer.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "deque.h"

struct deque {
    const void **items;
    ssize_t num_items;
    ssize_t capacity;
    ssize_t head;
    ssize_t one_past_tail;
};

static errno_t deque_increase_capacity(
    deque *deque);

errno_t deque_create(
    deque **out_deque) {
    deque *tmp = calloc(1, sizeof(deque));
    if (!tmp) {
        return ENOMEM;
    }

    *out_deque = tmp;

    return 0;
}

size_t deque_get_size(
    const deque *deque) {
    assert(deque != NULL);

    return deque->num_items;
}

errno_t deque_enqueue_front(
    deque *deque,
    const void *item) {
    assert(deque != NULL);

    // Resize if necessary.
    errno_t err = deque_increase_capacity(deque);
    if (err) return err;

    deque->head = (deque->head - 1 + deque->capacity) % deque->capacity;
    deque->items[deque->head] = item;
    deque->num_items++;

    return 0;
}

errno_t deque_enqueue_back(
    deque *deque,
    const void *item) {
    assert(deque != NULL);

    // Resize if necessary.
    errno_t err = deque_increase_capacity(deque);
    if (err) return err;

    deque->items[deque->one_past_tail] = item;
    deque->num_items++;
    deque->one_past_tail = (deque->one_past_tail + 1) % deque->capacity;

    return 0;
}

static errno_t deque_increase_capacity(
    deque *deque) {
    if (deque->num_items < deque->capacity) {
        return 0;
    }

    const ssize_t new_capacity = (deque->capacity > 0) ? (deque->capacity * 2) : 10;

    const void **tmp = calloc(1, new_capacity * sizeof(void *));
    if (!tmp) return ENOMEM;

    // Copy items into the new list, reset head and tail.
    for (ssize_t i = 0; i < deque->num_items; i++) {
        tmp[i] = deque->items[(deque->head + i) % deque->capacity];
    }

    free(deque->items);

    deque->items = tmp;
    deque->capacity = new_capacity;
    deque->head = 0;
    deque->one_past_tail = deque->num_items;

    return 0;
}

bool deque_peek_front(
    const deque *deque,
    const void **out_item) {
    assert(deque != NULL);

    if (deque->num_items == 0) {
        return false;
    }

    *out_item = deque->items[deque->head];

    return true;
}

bool deque_peek_back(
    const deque *deque,
    const void **out_item) {
    assert(deque != NULL);

    if (deque->num_items == 0) {
        return false;
    }

    *out_item = deque->items[deque->one_past_tail - 1];

    return true;
}

bool deque_pop_front(
    deque *deque,
    const void **out_item) {
    assert(deque != NULL);

    if (deque->num_items == 0) {
        return false;
    }

    *out_item = deque->items[deque->head];

    deque->head = (deque->head + 1) % deque->capacity;
    deque->num_items--;

    return true;
}

bool deque_pop_back(
    deque *deque,
    const void **out_item) {
    assert(deque != NULL);

    if (deque->num_items == 0) {
        return false;
    }

    const ssize_t target = (deque->one_past_tail - 1 + deque->capacity) % deque->capacity;

    *out_item = deque->items[target];

    deque->one_past_tail = target;
    deque->num_items--;

    return true;
}

void deque_free(
    deque *deque) {
    if (!deque) return;

    free(deque->items);
    free(deque);
}
