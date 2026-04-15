/**
 * string_util.c
 * -------------
 */

#include "string_util.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

errno_t string_concat(
    const char *first,
    const char *second,
    char **out) {
    assert(first != NULL);
    assert(second != NULL);

    const size_t len_first = strlen(first);
    const size_t len_second = strlen(second);

    char *combined = malloc(len_first + len_second + 1);
    if (!combined) {
        return ENOMEM;
    }

    memcpy(combined, first, len_first);
    memcpy(combined + len_first, second, len_second + 1);

    *out = combined;

    return 0;
}
