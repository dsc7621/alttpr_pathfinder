/**
 * file_util.c
 * -----------
 */

#include <assert.h>
#include <errno.h>

#include "file_util.h"

errno_t file_read_uint8(
    FILE *file,
    long offset,
    uint8_t *out) {
    assert(file !=NULL);

    int err = fseek(file, offset, SEEK_SET);
    if (err) {
        return err;
    }

    size_t bytes_read = fread(out, sizeof(uint8_t), 1, file);

    if (bytes_read != 1) {
        return EINVAL;
    }

    return 0;
}

errno_t file_read_le_uint16(
    FILE *file,
    long offset,
    uint16_t *out) {
    assert(file !=NULL);

    int err = fseek(file, offset, SEEK_SET);
    if (err) {
        return err;
    }

    uint16_t bytes[2];
    size_t bytes_read = fread(bytes, sizeof(uint8_t), 2, file);

    if (bytes_read != 2) {
        return EINVAL;
    }

    *out = bytes[0] | (bytes[1] << 8);

    return 0;
}
