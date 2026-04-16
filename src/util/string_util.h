/**
 * string_util.h
 * -------------
 * Provides utility functions for performing string operations.
 */

#ifndef ALTTPR_PATHFINDER_STRING_UTIL_H
#define ALTTPR_PATHFINDER_STRING_UTIL_H

/**
 * Concatenate the two given strings.
 *
 * @param first The first string.
 * @param second The second string.
 * @param out After execution, caller's variable will point to the concatenated string.
 * @return An error code:
 *     0 if successful,
 *     ENOMEM if memory allocation fails.
 *
 * @note The caller is responsible for freeing the resulting string.
 */
int string_concat(
    const char *first,
    const char *second,
    char **out);

#endif //ALTTPR_PATHFINDER_STRING_UTIL_H
