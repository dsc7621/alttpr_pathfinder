/**
 * world.h
 * -------
 * Provides the graph structure that represents a "world" of interconnected locations, as well as functions to
 * build a world from a ROM and safely clean it up.
 */

#ifndef ALTTPR_PATHFINDER_WORLD_H
#define ALTTPR_PATHFINDER_WORLD_H

#include <stdio.h>

/**
 * Represents a "world" of interconnected locations.
 */
typedef struct world world;

/**
 * Represents a location in a world.
 */
typedef struct location location;

/**
 * Build a world from the given ROM.
 *
 * @param out_world After execution, caller's variable will point to the world.
 * @param rom_filename The ROM filename.
 * @param rom A The Legend of Zelda: A Link to the Past (JP v1.0) ROM file.
 * @return 0 if successful; an error code, otherwise.
 *
 * @note If successful, the caller is responsible for calling <tt>world_free</tt> when done with the world.
 */
errno_t world_build(
    world **out_world,
    const char *rom_filename,
    FILE *rom);

/**
 * Get the filename of the ROM from which the world was built.
 *
 * @param world The world.
 * @return The filename of the ROM from which the world was built.
 */
const char *world_get_rom_filename(
    const world *world);

/**
 * Print the path between the starting location and target location.
 *
 * @param world The world.
 * @param start_location_name The name of the starting location.
 * @param target_location_name The name of the target location.
 * @return 0 if successful; an error code, otherwise.
 */
errno_t world_print_path(
    const world *world,
    const char *start_location_name,
    const char *target_location_name);

/**
 * Print the path between all four default starting locations (Link's House, the Sanctuary, the old man's house,
 * and the Pyramid) and the given target location.
 *
 * @param world The world.
 * @param target_location_name The name of the target location.
 * @return 0 if successful; an error code, otherwise.
 */
errno_t world_print_starter_paths(
    const world *world,
    const char *target_location_name);

/**
 * Frees the world, man. Be saved, sheeple.
 *
 * @param world The world.
 */
void world_free(
    world *world);

#endif // ALTTPR_PATHFINDER_WORLD_H
