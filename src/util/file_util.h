/**
 * file_util.h
 * -----------
 * Declares utility functions related to files.
 */

#ifndef ALTTPR_PATHFINDER_FILE_UTIL_H
#define ALTTPR_PATHFINDER_FILE_UTIL_H

#include <stdint.h>
#include <stdio.h>

/**
 * Read an 8-bit unsigned integer from the file at the given offset.
 *
 * @param file A pointer to the file.
 * @param offset The offset of the 8-bit unsigned integer to read, in bytes.
 * @param out After execution, will point to the value of the 8-bit unsigned integer at \c offset.
 * @return An error code:
 *     0 if successful,
 *     \c EINVAL if a byte cannot be read at position \c offset,
 *     another non-zero value if \c offset is invalid for \c file.
 */
errno_t file_read_uint8(
    FILE *file,
    long offset,
    uint8_t *out);

/**
 * Read a little-endian 16-bit unsigned integer from the file at the given offset.
 *
 * @param file A pointer to the file.
 * @param offset The offset of the 16-bit unsigned integer to read, in bytes.
 * @param out After execution, will point to the value of the 16-bit unsigned integer at \c offset.
 * @return An error code:
 *     0 if successful,
 *     \c EINVAL if two bytes cannot be read at position \c offset,
 *     another non-zero value if \c offset is invalid for \c file.
 */
errno_t file_read_le_uint16(
    FILE *file,
    long offset,
    uint16_t *out);

#endif // ALTTPR_PATHFINDER_FILE_UTIL_H
