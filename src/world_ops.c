/**
 * world_ops.c
 * -----------
 */

#include "deque.h"
#include "world.h"
#include "world_impl.h"
#include "util/macros.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct path_part {
    const char *traversal_method;
    const char *to_location_name;
    const char *to_location_placement;
} path_part;

static void print_path_parts(
    const path_part *path_parts,
    size_t num_path_parts);

int world_print_path(
    const world *world,
    const char *start_location_name,
    const char *target_location_name) {
    assert(world != NULL);
    assert(start_location_name != NULL);
    assert(target_location_name != NULL);

    int err = 0;

    const location *start_location = world_get_non_region_location(world, PLACEMENT_INTERIOR, start_location_name);
    if (!start_location) {
        start_location = world_get_non_region_location(world, PLACEMENT_EXTERIOR, start_location_name);

        if (!start_location) {
            printf("Cannot find location %s.\n", start_location_name);
            return EINVAL;
        }
    }

    const location *target_location = world_get_non_region_location(world, PLACEMENT_INTERIOR, target_location_name);
    if (!target_location) {
        target_location = world_get_non_region_location(world, PLACEMENT_EXTERIOR, target_location_name);

        if (!target_location) {
            printf("Cannot find location %s.\n", target_location_name);
            return EINVAL;
        }
    }

    deque *location_queue = NULL;
    deque *path_deque = NULL;
    path_part *path_parts = NULL;

    err = deque_create(&location_queue);
    if (err) goto cleanup;

    err = deque_enqueue_front(location_queue, start_location->world_offset);
    if (err) goto cleanup;

    bool locations_visited[512] = {0};

    size_t location_parents[512];
    for (size_t i = 0; i < 512; i++) {
        location_parents[i] = SIZE_MAX;
    }

    size_t path_finale = SIZE_MAX;

    // Vanilla BFS with "parent" tracking.
    while (deque_get_size(location_queue) > 0) {
        size_t location_offset;
        deque_pop_back(location_queue, &location_offset);

        // Find the location and mark it as visited.
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
                err = deque_enqueue_front(location_queue, world_offset);
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
            deque_enqueue_back(path_deque, path_curr);
            path_curr = location_parents[path_curr];
        }

        // ...then read it right back out of the back to reverse it.
        path_parts = calloc(deque_get_size(path_deque), sizeof(path_part));
        size_t num_path_parts = 0;

        const location *prev_location = NULL;
        while (deque_get_size(path_deque) > 0) {
            size_t location_offset;
            deque_pop_back(path_deque, &location_offset);

            const location *curr_location = world->locations[location_offset];

            // Build a list of path parts.
            path_part curr_path_part = {
                .to_location_name = curr_location->name,
                .to_location_placement = placement_to_str(curr_location->placement),
            };

            if (!prev_location) {
                // Traversal method is always just "Start" for our first point.
                curr_path_part.traversal_method = "Start";
            }
            else {
                // Otherwise determine the traversal method from the previous location by looking at its connections.
                // Traversal methods from locations to region-type locations stay null.
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
                    method = (curr_location->placement == PLACEMENT_REGION) ? NULL : "Go to";
                }

                curr_path_part.traversal_method = method;
            }

            path_parts[num_path_parts] = curr_path_part;
            num_path_parts++;

            prev_location = curr_location;
        }

        print_path_parts(path_parts, num_path_parts);
    }

cleanup:
    deque_free(location_queue);
    deque_free(path_deque);
    free(path_parts);
    return err;
}

int world_print_starter_paths(
    const world *world,
    const char *target_location_name) {
    assert(world != NULL);
    assert(target_location_name != NULL);

    int err;

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

/**
 * Actually pretty-prints a bunch of path segments.
 */
static void print_path_parts(
    const path_part *path_parts,
    const size_t num_path_parts) {
    if (path_parts == NULL) return;
    if (num_path_parts == 0) return;

    // Find the longest traversal method and location name used.
    size_t max_traversal_method_length = 0;
    size_t max_location_name_length = 0;

    for (size_t i = 0; i < num_path_parts; i++) {
        const path_part *curr_path_part = &path_parts[i];

        // Ignore NULL traversal methods for size considerations; this doesn't actually indicate traversal but
        // simply an acknowledgement that a location is in a region (and that region will be printed on the same
        // line).
        if (!curr_path_part->traversal_method) {
            continue;
        }

        const size_t traversal_method_length = strlen(curr_path_part->traversal_method);
        if (traversal_method_length > max_traversal_method_length) {
            max_traversal_method_length = traversal_method_length;
        }

        const size_t location_name_length =
            strlen(curr_path_part->to_location_name) + strlen(curr_path_part->to_location_placement) + 1;
        if (location_name_length > max_location_name_length) {
            max_location_name_length = location_name_length;
        }
    }

    // The starting path part prints in a dedicated format.
    char start_buf[512];
    snprintf(
        start_buf,
        sizeof(start_buf),
        "Start: %s %s",
        path_parts[0].to_location_placement,
        path_parts[0].to_location_name);

    // The +13 here is +4 characters for the indentation on subsequent lines, +5 characters for " --> " between traversal
    // method and location name, +4 for spaces between location name and region.
    printf(
        "\n%-*s",
        (int)max_traversal_method_length + (int)max_location_name_length + 13,
        start_buf);

    // Print the remainder of the path.
    for (size_t i = 1; i < num_path_parts; i++) {
        const path_part *curr_path_part = &path_parts[i];

        if (!curr_path_part->traversal_method) {
            printf("[in region %s]", curr_path_part->to_location_name);
        }
        else {
            char buf[512];
            snprintf(
                buf,
                sizeof(buf),
                "%s %s",
                curr_path_part->to_location_placement,
                curr_path_part->to_location_name);

            printf(
                "\n%*s --> %-*s",
                (int)max_traversal_method_length + 4,
                curr_path_part->traversal_method,
                (int)max_location_name_length + 4,
                buf);
        }
    }

    printf("\n");
}
