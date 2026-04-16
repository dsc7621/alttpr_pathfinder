/**
 * main.c
 * ------
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "locations.h"
#include "world.h"
#include "util/macros.h"

enum menu_choice {
    CHOICE_LOAD_ROM,
    CHOICE_STARTING_LOCATIONS_TO_GIVEN_TARGET,
    CHOICE_GIVEN_STARTING_LOCATION_TO_GIVEN_TARGET,
    CHOICE_LIST_LOCATIONS,
    CHOICE_QUIT,
};

static int try_build_world(
    const char *file_path,
    world **out_world);

static enum menu_choice print_menu_and_get_choice(
    const world *world);

static void fgets_with_retry(
    char *buf);

static void handle_load_rom(
    world **out_world);

static void handle_starting_locations_to_target(
    const world *world);

static void handle_given_location_to_given_target(
    const world *world);

static void handle_list_locations();

int main(
    int argc,
    char **argv) {
    int err = 0;
    world *world = NULL;

    // Handle single-run command-line mode.
    if (argc >= 3) {
        err = try_build_world(argv[1], &world);

        if (err) {
            DPRINTF("Got %d from try_build_world.\n", err);
            goto cleanup;
        }

        // Arbitrary start and end.
        if (argc >= 4) {
            err = world_print_path(world, argv[2], argv[3]);
            if (err) {
                DPRINTF("Got %d from world_print_path.\n", err);
                printf("Either the start or target location given is invalid.\n");
            }
            goto cleanup;
        }

        // Starting locations to end.
        err = world_print_starter_paths(world, argv[2]);
        if (err) {
            DPRINTF("Got %d from world_print_starter_paths.\n", err);
            printf("The target location given is invalid.\n");
        }
        goto cleanup;
    }

    // Handle menu mode.
    bool running = true;
    while (running) {
        const enum menu_choice choice = print_menu_and_get_choice(world);

        switch (choice) {
            case CHOICE_LOAD_ROM:
                handle_load_rom(&world);
                break;
            case CHOICE_STARTING_LOCATIONS_TO_GIVEN_TARGET:
                handle_starting_locations_to_target(world);
                break;
            case CHOICE_GIVEN_STARTING_LOCATION_TO_GIVEN_TARGET:
                handle_given_location_to_given_target(world);
                break;
            case CHOICE_LIST_LOCATIONS:
                handle_list_locations();
                break;
            default:
                printf("Bye!\n");
                running = false;
        }
    }

cleanup:
    world_free(world);
    return err;
}

/**
 * Load the file at the given path and try to build a world from it.
 */
static int try_build_world(
    const char *file_path,
    world **out_world) {
    int err;

    if (*out_world) {
        world_free(*out_world);
    }

    FILE *rom = fopen(file_path, "rb");
    if (rom) {
        err = world_build(out_world, file_path, rom);
        if (err) {
            DPRINTF("Got %d from world_build.\n", err);
        }

        fclose(rom);
    }
    else {
        err = EINVAL;
    }

    if (err) {
        printf(
            "Cannot build map of Hyrule from file at %s. "
            "Either your file path is incorrect, the file isn't a valid ALttP JP v1.0 ROM, or the ROM is randomized "
            "using currently unsupported options (likely inversion).\n",
            file_path);
        return err;
    }

    return 0;
}

/**
 * Print the menu and return the user's choice.
 */
static enum menu_choice print_menu_and_get_choice(
    const world *world) {
    if (world) {
        printf(
            "\n1.) Load a new ROM file (currently: %s).\n"
            "2.) Find paths from Link's possible starting locations to a target location.\n"
            "3.) Find a path between two arbitrary locations.\n"
            "4.) List locations.\n"
            "5.) Quit.\n> ",
            world_get_rom_filename(world));

        while (true) {
            char buf[1];

            const int num_read = scanf("%s", buf);
            if (num_read != 1) {
                printf("Try again.\n> ");
                continue;
            }

            switch (buf[0]) {
                case '1': return CHOICE_LOAD_ROM;
                case '2': return CHOICE_STARTING_LOCATIONS_TO_GIVEN_TARGET;
                case '3': return CHOICE_GIVEN_STARTING_LOCATION_TO_GIVEN_TARGET;
                case '4': return CHOICE_LIST_LOCATIONS;
                case '5': return CHOICE_QUIT;
                default: break;
            }

            printf("Try again: ");
        }
    }

    printf(
        "\n1.) Load a ROM file.\n"
        "2.) Quit.\n> ");

    while (true) {
        char buf[1];

        const int num_read = scanf("%s", buf);
        if (num_read != 1) {
            printf("Try again: ");
            continue;
        }

        switch (buf[0]) {
            case '1': return CHOICE_LOAD_ROM;
            case '2': return CHOICE_QUIT;
            default: break;
        }

        printf("Try again.\n> ");
    }
}

/**
 * Call fgets (with a 512-char limit) until it's successful. Strip the newline character.
 */
static void fgets_with_retry(
    char *buf) {
    const int err = fflush(stdin);
    if (err) {
        DPRINTF("Got %d from fflush.\n", err);

        // We'll just straight up exit if we can't flush stdin.
        exit(err);
    }

    bool trying = true;
    while (trying) {
        if (!fgets(buf, 512, stdin)) {
            printf("Try again: ");
            continue;
        }

        buf[strcspn(buf, "\n")] = '\0';
        trying = false;
    }
}

/**
 * Load a ROM file and build the world graph.
 */
static void handle_load_rom(
    world **out_world) {
    printf("\nEnter the path to the ROM file to load: ");

    char buf[512];
    fgets_with_retry(buf);

    bool trying = true;
    while (trying) {
        const int err = try_build_world(buf, out_world);
        if (err) {
            printf("Try again: ");
            fgets_with_retry(buf);
            continue;
        }

        printf("Successfully constructed world from ROM at %s.\n", buf);
        trying = false;
    }
}

/**
 * Calculate and print the paths from Link's four starting locations to the given target location.
 */
static void handle_starting_locations_to_target(
    const world *world) {
    printf("\nEnter the target location: ");

    char buf[512];
    fgets_with_retry(buf);

    printf("\n");
    world_print_starter_paths(world, buf);
}

/**
 * Calculate and print the path from the given starting location to the given target location.
 */
static void handle_given_location_to_given_target(
    const world *world) {
    printf("\nEnter the starting location: ");

    char start_buf[512];
    fgets_with_retry(start_buf);

    printf("Enter the target location: ");

    char target_buf[512];
    fgets_with_retry(target_buf);

    printf("\n");
    world_print_path(world, start_buf, target_buf);
}

/**
 * List all non-region locations (i.e., those valid for
 */
static void handle_list_locations() {
    for (size_t i = 0; i < num_entrances; i++) {
        printf("%s\n", entrances[i].name);
    }

    for (size_t i = 0; i < num_holes; i++) {
        printf("%s\n", holes[i].name);
    }
}
