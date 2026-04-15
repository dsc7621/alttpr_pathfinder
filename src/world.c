/**
 * world.c
 * -------
 * Reads a ROM file into a (currently unweighted) graph representing Hyrule. Uses a typical BFS to find paths
 * between locations.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "deque.h"
#include "locations.h"
#include "world.h"
#include "util/file_util.h"
#include "util/macros.h"

/**
 * Should be more than enough to cover everything at this point.
 */
static size_t default_locations_capacity = 512;

/**
 * Most locations only have one or two connections, but some regions will expand a few times.
 */
static const size_t default_connections_capacity = 2;

/**
 * Represents the placement of a <tt>location</tt>. It can be an interior location, exterior location, or a region
 * itself.
 */
enum placement {
    PLACEMENT_INTERIOR,
    PLACEMENT_EXTERIOR,
    PLACEMENT_REGION,
};

/**
 * Get a placement as a string literal.
 */
static const char *placement_to_str(
    enum placement placement) {
    switch (placement) {
        case PLACEMENT_INTERIOR: return "Inside";
        case PLACEMENT_EXTERIOR: return "Outside";
        case PLACEMENT_REGION: return "Region";
        default: return "Unknown";
    }
}

static const void *size_to_ptr(
    const size_t size) {
    return (const void *)(uintptr_t)size;
}

static size_t ptr_to_size(
    const void *ptr) {
    return (size_t)(uintptr_t)ptr;
}

/**
 * Represents a location in the game that is either just outside an exit or just inside an entrance.
 * Has connections to one or more other locations.
 */
typedef struct location location;

/**
 * Represents a unidirectional connection between two locations.
 */
typedef struct connection connection;

struct location {
    size_t world_offset;
    const char *name;
    enum placement placement;
    enum region region;
    size_t entrances_offset;

    connection *connections;
    size_t num_connections;
    size_t connections_capacity;
};

struct connection {
    const char *description;

    const location *to_location;
};

struct world {
    char *rom_filename;

    location **locations;
    size_t num_locations;
    size_t locations_capacity;

    const location *inside_links_house;
    const location *inside_sanctuary;
    const location *inside_old_man_cave;
    const location *on_pyramid;
};

/**
 * Determine whether a location has a connection that points to the same location as the given connection.
 */
static bool location_has_connection(
    const location *location,
    const connection connection) {
    assert(location != NULL);

    for (int i = 0; i < location->num_connections; i++) {
        if (location->connections[i].to_location == connection.to_location) {
            return true;
        }
    }

    return false;
}

/**
 * Add the given connection to a location. The location will allocate additional memory for connections if needed.
 */
static errno_t location_add_connection(
    location *location,
    const connection connection) {
    if (location_has_connection(location, connection)) {
        DPRINTF(
            "Ignoring duplicate connection %s %s -(%s)-> %s %s.\n",
            placement_to_str(location->placement),
            location->name,
            connection.description,
            placement_to_str(connection.to_location->placement),
            connection.to_location->name);
        return 0;
    }

    // Can't connect a location to itself.
    if (connection.to_location == location) {
        return EINVAL;
    }

    // Double the connection capacity, if needed.
    if (location->num_connections >= location->connections_capacity) {
        size_t new_capacity = location->connections_capacity
            ? location->connections_capacity * 2
            : default_connections_capacity;

        struct connection *tmp = realloc(location->connections, new_capacity * sizeof(struct connection));
        if (!tmp) {
            return ENOMEM;
        }

        location->connections = tmp;
        location->connections_capacity = new_capacity;
    }

    // Add the new connection.
    location->connections[location->num_connections] = connection;
    location->num_connections++;

    return 0;
}

/**
 * Free the memory used by the given location. Also frees all of its connections.
 */
static void location_free(
    location *location) {
    if (!location) {
        return;
    }

    free(location->connections);
    free(location);
}

/**
 * Determine whether a world has a location matching on placement, region, and name.
 */
static location *world_get_location(
    const world *world,
    const enum placement placement,
    const enum region region,
    const char *name) {
    for (size_t i = 0; i < world->num_locations; i++) {
        location *world_location = world->locations[i];

        if ((world_location->placement == placement) &&
            (world_location->region == region) &&
            (strcmp(world_location->name, name) == 0)) {
            return world_location;
        }
    }

    return NULL;
}

/**
 * Get a location that's known not to be a region (all non-region locations have unique placement-name pairs).
 */
static location *world_get_non_region_location(
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

/**
 * Like above, but specifically finds a region location.
 */
static location *world_get_region(
    const world *world,
    const enum region region) {
    for (size_t i = 0; i < world->num_locations; i++) {
        location *world_location = world->locations[i];

        if ((world_location->placement == PLACEMENT_REGION) &&
            (world_location->region == region)) {
            return world_location;
        }
    }

    return NULL;
}

// region World-building

static errno_t world_build_entrances_regions_and_exits(
    world *world,
    FILE *rom);

static errno_t world_build_exit(
    world *world,
    FILE *rom,
    location *interior_location,
    bool *out_is_unshuffled);

static errno_t world_build_holes(
    world *world,
    FILE *rom);

static errno_t world_build_region_links(
    world *world);

static void world_set_special_location(
    world *world,
    const location *location);

/**
 * Given all the relevant information for a location, add that location to the world. If it already exists in the
 * world, it won't be added. In either case, out_location will point to the location after execution.
 */
static errno_t world_add_or_get_location(
    world *world,
    const char *name,
    const enum placement placement,
    const enum region region,
    const size_t entrances_offset,
    location **out_location) {
    *out_location = world_get_location(world, placement, region, name);
    if (*out_location) {
        return 0;
    }

    // Double the location capacity, if needed.
    if (world->num_locations >= world->locations_capacity) {
        const size_t new_capacity = (world->locations_capacity > 0)
            ? world->locations_capacity * 2
            : default_locations_capacity;

        location **tmp = realloc(world->locations, new_capacity * sizeof(location*));
        if (!tmp) return ENOMEM;

        world->locations = tmp;
        world->locations_capacity = new_capacity;
    }

    // Create and add the new location.
    location *to_add = calloc(1, sizeof(location));
    if (!to_add) return ENOMEM;

    to_add->world_offset = world->num_locations;
    to_add->name = name;
    to_add->placement = placement;
    to_add->region = region;
    to_add->entrances_offset = entrances_offset;

    world->locations[world->num_locations] = to_add;
    world->num_locations++;

    *out_location = to_add;

    world_set_special_location(world, to_add);

    return 0;
}

errno_t world_build(
    world **out_world,
    const char *rom_filename,
    FILE *rom) {
    assert(rom_filename != NULL);
    assert(rom != NULL);

    *out_world = NULL;

    errno_t err;

    world *world = calloc(1, sizeof(struct world));
    if (!world) return ENOMEM;

    world->rom_filename = strdup(rom_filename);

    // Build entrances and exits.
    err = world_build_entrances_regions_and_exits(world, rom);
    if (err) goto cleanup;

    // Then build holes.
    err = world_build_holes(world, rom);
    if (err) goto cleanup;

    // Then region links.
    err = world_build_region_links(world);
    if (err) goto cleanup;

    // Success!
    *out_world = world;

    return 0;

cleanup:
    world_free(world);
    return err;
}

/**
 * Build the entrances, regions, and exits for a world.
 */
static errno_t world_build_entrances_regions_and_exits(
    world *world,
    FILE *rom) {
    errno_t err;

    bool entrances_used[256] = {0};

    for (size_t i = 0; i < num_entrances; i++) {
        const entrance *vanilla_entrance = &entrances[i];

        // Pluck the ROM's target value at the offset.
        uint8_t vanilla_entrance_target_value;
        err = file_read_uint8(rom, vanilla_entrance->target_offset, &vanilla_entrance_target_value);
        if (err) {
            DPRINTF(
                "Cannot read byte at offset 0x%x for entrance (with vanilla name) %s.\n",
                vanilla_entrance->target_offset,
                vanilla_entrance->name);
            return err;
        }

        // Now, loop through the entrances again, and find the entrance whose VANILLA target matches the value
        // plucked from the ROM.
        const entrance *rom_entrance = NULL;
        size_t rom_entrance_offset = SIZE_MAX;

        for (size_t j = 0; j < num_entrances; j++ ) {
            // Skip if we've already used this entrance (this is to accommodate the fact that I
            // cannot currently tell the difference between various shops, fairy caves, etc.).
            if (entrances_used[j]) {
                continue;
            }

            const entrance *temp_entrance = &entrances[j];

            if (temp_entrance->vanilla_target_value == vanilla_entrance_target_value) {
                rom_entrance = temp_entrance;
                rom_entrance_offset = j;

                entrances_used[j] = true;
                break;
            }
        }

        if (!rom_entrance) {
            DPRINTF("Cannot find target for entrance (with vanilla name) %s. Bad ROM, perhaps?\n", vanilla_entrance->name);
            return EINVAL;
        }

        // We now have two entrances:
        //     vanilla_entrance <-- represents the EXPECTED location (e.g., Inside Misery Mire)
        //     rom_entrance     <-- represents the ACTUAL ROM location (e.g., Inside Tower of Hera)
        //
        // This means that when we enter into the front door of Misery Mire in the overworld, the actual interior
        // location is Tower of Hera (and not what we'd expect, which is, of course, the interior of Misery Mire).
        //
        // We can now create two locations: "Outside Misery Mire" and "Inside Tower of Hera" and draw a connection
        // between them.
        location *entrance_outside = NULL;
        err = world_add_or_get_location(
            world, vanilla_entrance->name,
            PLACEMENT_EXTERIOR,
            vanilla_entrance->vanilla_exterior_region,
            rom_entrance_offset,
            &entrance_outside);
        if (err) return err;

        location *entrance_inside = NULL;
        err = world_add_or_get_location(
            world,
            rom_entrance->name,
            PLACEMENT_INTERIOR,
            rom_entrance->vanilla_interior_region,
            rom_entrance_offset,
            &entrance_inside);
        if (err) return err;

        // Make the connection, then add it to the exterior location.
        const connection entrance_connection = {
            .description = "Enter",
            .to_location = entrance_inside,
        };

        err = location_add_connection(entrance_outside, entrance_connection);
        if (err) return err;

        DPRINTF("Added Outside %s --> Inside %s\n", vanilla_entrance->name, rom_entrance->name);

        // Next, create and add the regions for the outside, and, if necessary, the inside, and add connections to and
        // from them. Location <-> Region are the only bidirectional connections we add.
        location *region_outside = NULL;
        err = world_add_or_get_location(
            world,
            region_to_str(entrance_outside->region),
            PLACEMENT_REGION,
            entrance_outside->region,
            SIZE_MAX,
            &region_outside);
        if (err) return err;

        const connection entrance_to_region_outside = {
            .description = NULL,
            .to_location = region_outside,
        };
        err = location_add_connection(entrance_outside, entrance_to_region_outside);
        if (err) return err;

        const connection region_to_entrance_outside = {
            .description = NULL,
            .to_location = entrance_outside,
        };
        err = location_add_connection(region_outside, region_to_entrance_outside);
        if (err) return err;

        DPRINTF("Added Outside %s <--> %s\n", vanilla_entrance->name, region_outside->name);

        // Don't do connections to/from isolated interior regions. There's no point.
        if (entrance_inside->region != REGION_INT_ISOLATED) {
            location *region_inside = NULL;
            err = world_add_or_get_location(
                world,
                region_to_str(entrance_inside->region),
                PLACEMENT_REGION,
                entrance_inside->region,
                SIZE_MAX,
                &region_inside);
            if (err) return err;

            const connection entrance_to_region_inside = {
                .description = NULL,
                .to_location = region_inside,
            };
            err = location_add_connection(entrance_inside, entrance_to_region_inside);
            if (err) return err;

            const connection region_to_entrance_inside = {
                .description = NULL,
                .to_location = entrance_inside,
            };
            err = location_add_connection(region_inside, region_to_entrance_inside);
            if (err) return err;

            DPRINTF("Added Inside %s <--> %s\n", rom_entrance->name, region_inside->name);
        }

        // Next, add the exit for the new entrance.
        bool exit_is_unshuffled = false;
        err = world_build_exit(world, rom, entrance_inside, &exit_is_unshuffled);
        if (err) return err;

        // Finally, if the exit was unshuffled, we need to add another connection back out to the exterior.
        if (exit_is_unshuffled) {
            const connection entrance_inside_to_entrance_outside = {
                .description = "Leave",
                .to_location = entrance_outside,
            };
            err = location_add_connection(entrance_inside, entrance_inside_to_entrance_outside);
            if (err) return err;

            DPRINTF("Added Inside %s --> %s\n", rom_entrance->name, vanilla_entrance->name);
        }
    }

    return 0;
}

/**
 * Build the exit for a given entrance.
 */
static errno_t world_build_exit(
    world *world,
    FILE *rom,
    location *interior_location,
    bool *out_is_unshuffled) {
    errno_t err;

    // Check to see if there's an exit linked with the interior location.
    size_t interior_exits_offset = 0;
    bool link_found = false;

    for (size_t j = 0; j < num_location_links; j++) {
        const location_link *location_link = &location_links[j];

        if (location_link->entrances_offset == interior_location->entrances_offset) {
            interior_exits_offset = location_link->interior_exits_offset;
            link_found = true;
            break;
        }
    }

    // If we DON'T find a paired exit, it's normal. It means the interior's exit can't be shuffled, so it exits
    // to the same exterior location. We'll let the caller know to build the connection right back out to the
    // exterior.
    if (!link_found) {
        *out_is_unshuffled = true;
        return 0;
    }

    const interior_exit *vanilla_interior_exit = &interior_exits[interior_exits_offset];

    // Read the ROM's VRAM location value (this acts as a unique identifier for the exit) at the vanilla exit's offset.
    uint16_t vanilla_target_vram_location_value;
    err = file_read_le_uint16(
        rom,
        vanilla_interior_exit->target_vram_location_offset,
        &vanilla_target_vram_location_value);
    if (err) {
        DPRINTF(
            "Cannot read little-endian 16-bit value at offset 0x%x for exit (with vanilla name) %s.\n",
            vanilla_interior_exit->target_vram_location_offset,
            vanilla_interior_exit->name);
        return err;
    }

    // Now find the exit whose vanilla VRAM location value matches what the ROM's VRAM location value is for
    // the paired exit we found. That's the true target exit.
    const interior_exit *rom_interior_exit = NULL;
    for (size_t j = 0; j < num_interior_exits; j++) {
        const interior_exit *temp_interior_exit = &interior_exits[j];

        if (temp_interior_exit->vanilla_target_vram_location_value == vanilla_target_vram_location_value) {
            rom_interior_exit = temp_interior_exit;
            break;
        }
    }

    if (!rom_interior_exit) {
        DPRINTF(
            "Cannot find exit with vanilla target VRAM location value 0x%x. Bad ROM, maybe?\n",
            vanilla_target_vram_location_value);
        return EINVAL;
    }

    // We now have two exits:
    //     vanilla_interior_exit <-- represents the EXPECTED location (e.g., Outside Misery Mire)
    //     rom_interior_exit     <-- represents the ACTUAL ROM location (e.g., Outside Bonk Rock Cave)
    //
    // This means what when we exit Misery Mire, the actual exterior location is just outside of Bonk Rock Cave
    // (and not what we'd expect, which is, of course, just outside of Misery Mire).
    //
    // We can create a new connection between "Inside Misery Mire" and "Outside Bonk Rock Cave." We may need to
    // create the exterior location first.
    location *exterior_location = NULL;
    err = world_add_or_get_location(
        world,
        rom_interior_exit->name,
        PLACEMENT_EXTERIOR,
        rom_interior_exit->vanilla_exterior_region,
        SIZE_MAX,
        &exterior_location);
    if (err) {
        return err;
    }

    connection exit_connection = {
        .description = "Leave",
        .to_location = exterior_location,
    };
    err = location_add_connection(interior_location, exit_connection);
    if (err) {
        return err;
    }

    DPRINTF("Added Inside %s --> Outside %s\n", interior_location->name, exterior_location->name);

    return 0;
}

/**
 * Build the holes for a world.
 */
static errno_t world_build_holes(
    world *world,
    FILE *rom) {
    errno_t err;

    for (size_t i = 0; i < num_holes; i++) {
        const hole *vanilla_hole = &holes[i];

        // Pluck the ROM's target value at the first offset (this is always set).
        uint8_t vanilla_hole_target_value;
        err = file_read_uint8(rom, vanilla_hole->target_offsets[0], &vanilla_hole_target_value);
        if (err) {
            DPRINTF(
                "Cannot read byte at offset 0x%x for hole (with vanilla name) %s.\n",
                vanilla_hole->target_offsets[0],
                vanilla_hole->name);
            return err;
        }

        // Now loop through the holes again and find the one whose VANILLA target value matches the one plucked
        // from the ROM.
        const hole *rom_hole = NULL;
        for (size_t j = 0; j < num_holes; j++) {
            const hole *temp_hole = &holes[j];

            if (temp_hole->vanilla_target_value == vanilla_hole_target_value) {
                rom_hole = temp_hole;
                break;
            }
        }

        if (!rom_hole) {
            DPRINTF("Cannot find target for hole (with vanilla name) %s. Bad ROM, perhaps?\n", vanilla_hole->name);
            return EINVAL;
        }

        // We now have two holes (don't you dare.):
        //     vanilla_hole <-- represents the EXPECTED hole (e.g., Bat Cave Drop)
        //     rom_hole     <-- represents the ACTUAL ROM hole (e.g., Pyramid Hole)
        //
        // This means that when we jump into the Bat Cave hole just outside Kakariko, we'll actually end up inside
        // the pyramid in front of Ganon.
        //
        // We can now create one new location: "Outside Bat Cave Drop" and draw a new connection to the Pyramid
        // Hole's target (which is technically "Pyramid Escape").
        location *hole_location = NULL;
        err = world_add_or_get_location(
            world,
            vanilla_hole->name,
            PLACEMENT_EXTERIOR,
            vanilla_hole->vanilla_region,
            SIZE_MAX,
            &hole_location);
        if (err) {
            return err;
        }

        // Find the location (which will all exist by this point) to which to draw a connection.
        const location *target_location = NULL;
        for (size_t j = 0; j < world->num_locations; j++) {
            const location *temp_location = world->locations[j];

            // Holes only drop to interiors.
            if (temp_location->placement != PLACEMENT_INTERIOR) {
                continue;
            }

            if (strcmp(temp_location->name, rom_hole->vanilla_target_name) == 0) {
                target_location = temp_location;
                break;
            }
        }

        if (!target_location) {
            DPRINTF("Cannot find world location with name %s.\n", rom_hole->vanilla_target_name);
            return EINVAL;
        }

        // Add the connection to the new location.
        connection hole_connection = {
            .description = "Fall in",
            .to_location = target_location,
        };
        err = location_add_connection(hole_location, hole_connection);
        if (err) return err;

        DPRINTF("Added hole Outside %s --> Inside %s\n", hole_location->name, target_location->name);

        // Add region connections. All regions will exist at this point.
        location *hole_region = world_get_region(world, vanilla_hole->vanilla_region);

        connection hole_to_region = {
            .description = NULL,
            .to_location = hole_region,
        };
        err = location_add_connection(hole_location, hole_to_region);
        if (err) return err;

        connection region_to_hole = {
            .description = NULL,
            .to_location = hole_location,
        };
        err = location_add_connection(hole_region, region_to_hole);
        if (err) return err;
    }

    return 0;
}

/**
 * Build the direct region-to-region connections.
 */
static errno_t world_build_region_links(
    world *world) {
    for (size_t i = 0; i < num_region_links; i++) {
        const region_link *region_link = &region_links[i];

        location *origin_location = world_get_region(world, region_link->origin_region);
        if (!origin_location) {
            DPRINTF("Cannot find location for region %s.\n", region_to_str(region_link->origin_region));
            return EINVAL;
        }

        location *destination_location = world_get_region(world, region_link->destination_region);
        if (!destination_location) {
            DPRINTF("Cannot find location for region %s.\n", region_to_str(region_link->destination_region));
            return EINVAL;
        }

        const connection region_connection = {
            .description = region_link->method,
            .to_location = destination_location,
        };
        errno_t err = location_add_connection(origin_location, region_connection);
        if (err) return err;

        DPRINTF("Added %s --[%s]-> %s\n", origin_location->name, region_link->method, destination_location->name);
    }

    return 0;
}

/**
 * Checks a location against four special known starting locations for Link, and sets any found on the world.
 */
static void world_set_special_location(
    world *world,
    const location *location) {
    if (world->inside_links_house &&
        world->inside_sanctuary &&
        world->inside_old_man_cave &&
        world->on_pyramid) {
        return;
    }

    if ((strcmp(location->name, "Links House") == 0) && (location->placement == PLACEMENT_INTERIOR)) {
        world->inside_links_house = location;
        return;
    }

    if ((strcmp(location->name, "Sanctuary") == 0) && (location->placement == PLACEMENT_INTERIOR)) {
        world->inside_sanctuary = location;
        return;
    }

    if ((strcmp(location->name, "Old Man House (Top)") == 0) && (location->placement == PLACEMENT_INTERIOR)) {
        world->inside_old_man_cave = location;
        return;
    }

    if ((strcmp(location->name, "Pyramid Hole") == 0) && (location->placement == PLACEMENT_EXTERIOR)) {
        world->on_pyramid = location;
    }
}

// endregion World-building

const char *world_get_rom_filename(
    const world *world) {
    assert(world != NULL);

    return world->rom_filename;
}

// region World-printing

errno_t world_print_path(
    const world *world,
    const char *start_location_name,
    const char *target_location_name) {
    assert(world != NULL);
    assert(start_location_name != NULL);
    assert(target_location_name != NULL);

    errno_t err = 0;

    const location *start_location = world_get_non_region_location(world, PLACEMENT_INTERIOR, start_location_name);
    if (!start_location) {
        start_location = world_get_non_region_location(world, PLACEMENT_EXTERIOR, start_location_name);

        if (!start_location) {
            DPRINTF("Cannot find location %s.\n", start_location_name);
            return EINVAL;
        }
    }

    const location *target_location = world_get_non_region_location(world, PLACEMENT_INTERIOR, target_location_name);
    if (!target_location) {
        target_location = world_get_non_region_location(world, PLACEMENT_EXTERIOR, target_location_name);

        if (!target_location) {
            DPRINTF("Cannot find location %s.\n", target_location_name);
            return EINVAL;
        }
    }

    deque *location_queue = NULL;
    deque *path_deque = NULL;

    err = deque_create(&location_queue);
    if (err) goto cleanup;

    err = deque_enqueue_front(location_queue, size_to_ptr(start_location->world_offset));
    if (err) goto cleanup;

    bool locations_visited[512] = {0};

    size_t location_parents[512];
    for (size_t i = 0; i < 512; i++) {
        location_parents[i] = SIZE_MAX;
    }

    size_t path_finale = SIZE_MAX;

    // Vanilla BFS with "parent" tracking.
    while (deque_get_size(location_queue) > 0) {
        const void *tmp;
        deque_pop_back(location_queue, &tmp);

        // Find the location and mark it as visited.
        const size_t location_offset = ptr_to_size(tmp);
        const location *location = world->locations[location_offset];
        locations_visited[location_offset] = true;

        DPRINTF("Visiting %s %s\n", placement_to_str(location->placement), location->name);

        if (location == target_location) {
            path_finale = location_offset;
            break;
        }

        // Enqueue each of its connections.
        for (size_t i = 0; i < location->num_connections; i++) {
            const connection *connection = &location->connections[i];
            const size_t world_offset = connection->to_location->world_offset;

            if (!locations_visited[world_offset]) {
                err = deque_enqueue_front(location_queue, size_to_ptr(world_offset));
                if (err) goto cleanup;

                location_parents[world_offset] = location_offset;
            }
        }
    }

    if (path_finale == SIZE_MAX) {
        printf("Path from %s to %s not found.\n", start_location_name, target_location_name);
    }
    else {
        // Path reconstruction. This path will be reversed, since we track it from the ending location.
        // Place it into the back of a deque...
        err = deque_create(&path_deque);
        if (err) goto cleanup;

        size_t path_curr = path_finale;
        while (path_curr != SIZE_MAX) {
            deque_enqueue_back(path_deque, size_to_ptr(path_curr));
            path_curr = location_parents[path_curr];
        }

        // ...then read it right back out of the back to reverse it.
        const location *prev_location = NULL;
        while (deque_get_size(path_deque) > 0) {
            const void *tmp;
            deque_pop_back(path_deque, &tmp);

            const size_t location_offset = ptr_to_size(tmp);
            const location *curr_location = world->locations[location_offset];

            // Build and write the actual path text.
            if (!prev_location) {
                printf("Start: %s %s\n", placement_to_str(curr_location->placement), curr_location->name);
            }
            else {
                const char *method = NULL;
                for (size_t i = 0; i < prev_location->num_connections; i++) {
                    const connection *connection = &prev_location->connections[i];

                    if (connection->to_location != curr_location) {
                        continue;
                    }

                    method = connection->description;
                    break;
                }

                if (!method) {
                    method = (curr_location->placement == PLACEMENT_REGION) ? "in" : "go";
                }

                char prev_location_buf[64];
                snprintf(
                    prev_location_buf,
                    sizeof(prev_location_buf),
                    "%s %s",
                    placement_to_str(prev_location->placement),
                    prev_location->name);

                char curr_location_buf[64];
                snprintf(
                    curr_location_buf,
                    sizeof(curr_location_buf),
                    "%s %s",
                    placement_to_str(curr_location->placement),
                    curr_location->name);

                printf(
                    "%-64s -----> %-30s -----> %-64s\n",
                    prev_location_buf,
                    method,
                    curr_location_buf);
            }

            prev_location = curr_location;
        }
    }

cleanup:
    deque_free(location_queue);
    deque_free(path_deque);
    return err;
}

errno_t world_print_starter_paths(
    const world *world,
    const char *target_location_name) {
    assert(world != NULL);
    assert(target_location_name != NULL);

    errno_t err;

    err = world_print_path(world, world->inside_links_house->name, target_location_name);
    if (err) return err;

    printf("\n\n");

    err = world_print_path(world, world->inside_sanctuary->name, target_location_name);
    if (err) return err;

    printf("\n\n");

    err = world_print_path(world, world->inside_old_man_cave->name, target_location_name);
    if (err) return err;

    printf("\n\n");

    err = world_print_path(world, world->on_pyramid->name, target_location_name);
    if (err) return err;

    return 0;
}

// endregion World-printing

void world_free(
    world *world) {
    if (!world) {
        return;
    }

    free(world->rom_filename);

    for (size_t i = 0; i < world->num_locations; i++) {
        location_free(world->locations[i]);
    }

    free(world->locations);
    free(world);
}
