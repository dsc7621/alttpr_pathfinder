/**
 * deque.h
 * -------
 * Provides an interface for creating and interacting with a typical deque that stores size_t values.
 */

#ifndef ALTTPR_PATHFINDER_DEQUE_H
#define ALTTPR_PATHFINDER_DEQUE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A typical deque.
 */
typedef struct deque deque;

/**
 * Create a new deque.
 *
 * @param out_deque After successful execution, will point to a pointer to the new deque.
 * @return An error code:
 *     0 if successful,
 *     ENOMEM if space for the deque cannot be allocated.
 *
 * @remark Caller is responsible for calling deque_free when done with the deque.
 */
int deque_create(
    deque **out_deque);

/**
 * Get the number of elements in the deque.
 *
 * @param deque The deque.
 * @return The number of elements in the deque.
 */
size_t deque_get_size(
    const deque *deque);

/**
 * Enqueue new item into the front of the deque.
 *
 * @param deque The deque.
 * @param item The item to enqueue.
 * @return An error code:
 *     0 if successful,
 *     ENOMEM if space for the additional item cannot be allocated.
 */
int deque_enqueue_front(
    deque *deque,
    size_t item);

/**
 * Enqueue new item into the back of the deque.
 *
 * @param deque The deque.
 * @param item The item to enqueue.
 * @return An error code:
 *     0 if successful,
 *     ENOMEM if space for the additional item cannot be allocated.
 */
int deque_enqueue_back(
    deque *deque,
    size_t item);

/**
 * Look at the value at the front of the deque without removing it.
 *
 * @param deque The deque.
 * @param out_item After successful execution, will point to the value of the item at the front of the deque.
 * @return true if the deque contained an item to peek at; false, otherwise.
 */
bool deque_peek_front(
    const deque *deque,
    size_t *out_item);

/**
 * Look at the value at the back of the deque without removing it.
 *
 * @param deque The deque.
 * @param out_item After successful execution, will point to the value of the item at the back of the deque.
 * @return true if the deque contained an item to peek at; false, otherwise.
 */
bool deque_peek_back(
    const deque *deque,
    size_t *out_item);

/**
 * Look at and remove the value at the front of the deque.
 *
 * @param deque The deque.
 * @param out_item After successful execution, will point to the value of the item removed from the front of the deque.
 * @return true if the deque contained an item to pop; false, otherwise.
 */
bool deque_pop_front(
    deque *deque,
    size_t *out_item);

/**
 * Look at and remove the value at the back of the deque.
 *
 * @param deque The deque.
 * @param out_item After successful execution, will point to the value of the item removed from the back of the deque.
 * @return true if the deque contained an item to pop; false, otherwise.
 */
bool deque_pop_back(
    deque *deque,
    size_t *out_item);

/**
 * Frees up resources allocated for the deque.
 *
 * @param deque The queue.
 */
void deque_free(
    deque *deque);

#endif // ALTTPR_PATHFINDER_DEQUE_H
