/**
 * world_impl.h
 * ------------
 * Internal implementation of shared world types and functions.
 */

#ifndef ALTTPR_PATHFINDER_WORLD_INTERNAL_H
#define ALTTPR_PATHFINDER_WORLD_INTERNAL_H

#include "locations.h"
#include "world.h"

/**
 * Represents the placement of a location. It can be an interior location, exterior location, or a region
 * itself.
 */
enum placement {
    PLACEMENT_INTERIOR,
    PLACEMENT_EXTERIOR,
    PLACEMENT_REGION,
};

/**
 * Gets a placement as a string literal.
 *
 * @param placement The placement.
 * @return The placement as a string literal.
 */
const char *placement_to_str(
    enum placement placement);

/**
 * Represents a location in the game that is either just outside an exit or just inside an entrance.
 * Has connections to one or more other locations.
 */
typedef struct location location;

/**
 * Represents a unidirectional connection between two locations.
 */
typedef struct connection connection;

/**
 * Represents a location (a graph node, essentially).
 */
struct location {
    /**
     * The unique identifier of the location, the offset in the world that owns it.
     */
    size_t world_offset;

    /**
     * The location name.
     */
    const char *name;

    /**
     * The location's placement (inside, outside, or region).
     */
    enum placement placement;

    /**
     * The region in which the location is found (if it's inside or outside), or the region that it represents, if it
     * is itself a region.
     */
    enum region region;

    /**
     * The location's offset in the entrances list declared in locations.h. If location is a hole or region, this
     * value will be SIZE_MAX and serve no purpose.
     */
    size_t entrances_offset;

    /**
     * The location's connections to other locations.
     */
    connection *connections;

    /**
     * The current number of connections in connections.
     */
    size_t num_connections;

    /**
     * The current maximum capacity of connections.
     */
    size_t connections_capacity;
};

/**
 * Represents a connection to a location. The "from" location is the location that owns the connection.
 */
struct connection {
    /**
     * A short description of how the traversal to the target location is performed (
     */
    const char *description;

    /**
     * A pointer to the target location.
     */
    const location *to_location;
};

struct world {
    /**
     * The filename of the ROM from which the world is built.
     */
    char *rom_filename;

    /**
     * A list of pointers to locations in the world.
     */
    location **locations;

    /**
     * The current number of locations in locations.
     */
    size_t num_locations;

    /**
     * The current maximum capacity of locations.
     */
    size_t locations_capacity;

    /**
     * A pointer to the location inside Link's House.
     */
    const location *inside_links_house;

    /**
     * A pointer to the location inside the Sanctuary.
     */
    const location *inside_sanctuary;

    /**
     * A pointer to the location inside the Old Man's Cave, technically the "Old Man House (West)" location.
     */
    const location *inside_old_man_cave;

    /**
     * A pointer to the location on top of the Dark World Pyramid, technically the "Pyramid Hole" location.
     */
    const location *on_pyramid;
};

/**
 * Get a non-region location from a world based on its placement and name.
 *
 * @param world The world.
 * @param placement The location placement.
 * @param name The name of the location.
 * @return A pointer to the non-region location matching on placement and name, or NULL if no such location exists.
 */
const location *world_get_non_region_location(
    const world *world,
    enum placement placement,
    const char *name);

#endif //ALTTPR_PATHFINDER_WORLD_INTERNAL_H
