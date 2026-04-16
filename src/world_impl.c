/**
 * world_impl.c
 * ------------
 */

#include "world_impl.h"

#include <string.h>

const char *placement_to_str(
    const enum placement placement) {
    switch (placement) {
        case PLACEMENT_INTERIOR: return "Inside";
        case PLACEMENT_EXTERIOR: return "Outside";
        case PLACEMENT_REGION: return "Region";
        default: return "Unknown";
    }
}

const location *world_get_non_region_location(
    const world *world,
    const enum placement placement,
    const char *name) {
    for (size_t i = 0; i < world->num_locations; i++) {
        location *world_location = world->locations[i];

        if ((world_location->placement == placement) &&
            (strcmp(world_location->name, name) == 0)) {
            return world_location;
            }
    }

    return NULL;
}
