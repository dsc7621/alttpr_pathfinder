/**
 * macros.h
 * --------
 */

#ifndef ALTTPR_PATHFINDER_MACROS_H
#define ALTTPR_PATHFINDER_MACROS_H

// Debug print macro.
#ifdef DEBUG
#include <stdio.h>
#define DPRINTF(fmt, ...) \
    do { \
        printf("[DEBUG] %s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)
#else
#define DPRINTF(...) ((void)0)
#endif

#endif //ALTTPR_PATHFINDER_MACROS_H
