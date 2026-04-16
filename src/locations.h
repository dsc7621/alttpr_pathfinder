/**
 * locations.h
 * -----------
 * Defines types representing locations (regions, entrances, exits, and holes, as well as declares lists containing
 * information about those things sourced from a vanilla (i.e., unpatched) The Legend of Zelda: A Link to the Past
 * Japanese v1.0 ROM.
 */

#ifndef ALTTPR_PATHFINDER_LOCATIONS_H
#define ALTTPR_PATHFINDER_LOCATIONS_H

#include <stdint.h>

/**
 * The various exterior regions in the game. A region is contiguous area separated from other regions by entrances,
 * although several exterior regions allow one-way traversal to other exterior regions (e.g., jumping off the Light
 * World Death Mountain Return Ledge region into the general Light World region).
 *
 * @remark Abbreviations here: EXT (exterior), INT (interior), LW (Light World),
 * DW (Dark World).
 *
 * @remark It is currently assumed that regions are freely traversable, even though this is sometimes
 * limited by items (e.g., needing a hammer and/or hookshot and/or tier-two glove to move freely through
 * the main "Dark World" region).
 */
enum region {
    // Exterior Light World regions

    REGION_EXT_LW_OVERWORLD,
    REGION_EXT_LW_DEATH_MOUNTAIN_LOWER,
    REGION_EXT_LW_DEATH_MOUNTAIN_UPPER,
    REGION_EXT_LW_DEATH_MOUNTAIN_SPECTACLE_ROCK_CAVE_LOWER,
    REGION_EXT_LW_DEATH_MOUNTAIN_SPECTACLE_ROCK_CAVE_MID,
    REGION_EXT_LW_DEATH_MOUNTAIN_SPIRAL_CAVE_TOP,
    REGION_EXT_LW_DEATH_MOUNTAIN_FAIRY_ASCENSION_CAVE_TOP,
    REGION_EXT_LW_DEATH_MOUNTAIN_MIMIC_CAVE_LEDGE,
    REGION_EXT_LW_DEATH_MOUNTAIN_RETURN_LEDGE,
    REGION_EXT_LW_LOST_WOODS,
    REGION_EXT_LW_HYRULE_CASTLE_ROOF,
    REGION_EXT_LW_DESERT_PALACE_LEDGE,
    REGION_EXT_LW_DESERT_PALACE_EAST,
    REGION_EXT_LW_CAVE_45_LEDGE,
    REGION_EXT_LW_CHECKERBOARD_CAVE_LEDGE,
    REGION_EXT_LW_GRAVEYARD_CAVE_LEDGE,

    // Exterior Dark World regions

    REGION_EXT_DW_OVERWORLD,
    REGION_EXT_DW_MIRE,
    REGION_EXT_DW_BUMPER_CAVE_TOP,
    REGION_EXT_DW_DEATH_MOUNTAIN_LOWER_EAST,
    REGION_EXT_DW_DEATH_MOUNTAIN_LOWER_WEST,
    REGION_EXT_DW_DEATH_MOUNTAIN_UPPER,
    REGION_EXT_DW_DEATH_MOUNTAIN_FLOATING_LEDGE,
    REGION_EXT_DW_DEATH_MOUNTAIN_TURTLE_ROCK_LONG_LEDGE,
    REGION_EXT_DW_DEATH_MOUNTAIN_TURTLE_ROCK_ISOLATED_LEDGE,
    REGION_EXT_DW_DEATH_MOUNTAIN_SPIKE_CAVE_LEDGE,
    REGION_EXT_DW_LOST_WOODS_SOUTH,
    REGION_EXT_DW_LOST_WOODS_NORTH,
    REGION_EXT_DW_OUTSIDE_ICE_PALACE,

    // Interior Light World regions

    REGION_INT_LW_ELDER_HOUSE,
    REGION_INT_LW_TWO_BROTHERS_HOUSE,
    REGION_INT_LW_DEATH_MOUNTAIN_OLD_MAN_CAVE,
    REGION_INT_LW_DEATH_MOUNTAIN_OLD_MAN_HOUSE,
    REGION_INT_LW_DEATH_MOUNTAIN_RETURN_CAVE,
    REGION_INT_LW_DEATH_MOUNTAIN_PARADOX_CAVE,
    REGION_INT_LW_DEATH_MOUNTAIN_SPECTACLE_ROCK_CAVE_LOWER,
    REGION_INT_LW_DEATH_MOUNTAIN_SPECTACLE_ROCK_CAVE_MID,
    REGION_INT_LW_DEATH_MOUNTAIN_SPECTACLE_ROCK_CAVE_PEAK,
    REGION_INT_LW_DEATH_MOUNTAIN_SPIRAL_CAVE_UPPER,
    REGION_INT_LW_DEATH_MOUNTAIN_SPIRAL_CAVE_LOWER,
    REGION_INT_LW_DEATH_MOUNTAIN_FAIRY_ASCENSION_CAVE,
    REGION_INT_LW_HYRULE_CASTLE,
    REGION_INT_LW_SANCTUARY,
    REGION_INT_LW_DESERT_PALACE_FIRST_PART,

    // Interior Dark World regions

    REGION_INT_DW_BUMPER_CAVE,
    REGION_INT_DW_DEATH_MOUNTAIN_HOOKSHOT_CAVE,
    REGION_INT_DW_DEATH_MOUNTAIN_SUPERBUNNY_CAVE_LOWER,
    REGION_INT_DW_DEATH_MOUNTAIN_SUPERBUNNY_CAVE_UPPER,
    REGION_INT_DW_SKULL_WOODS_SECOND_SECTION,
    REGION_INT_DW_TURTLE_ROCK,

    // Special regions

    /**
     * The "isolated" interior region is special. It refers to any interior that only has a single exit and cannot
     * transition to any other interior region without passing through an exterior region.
     *
     * It's not necessary to create specific interior regions for every such interior, because it has no value for
     * pathfinding. This prevents this enum from being filled with dozens of useless values for specific buildings
     * or caves.
     *
     * The vast majority of interior regions are isolated.
     */
    REGION_INT_ISOLATED,
};

/**
 * Get the string name for the given region.
 *
 * @param region The region.
 * @return The region as a string.
 */
extern const char *region_to_str(
    enum region region);

/**
 * Some holes have two target offsets; for holes that only have one, the second offset is set to this value.
 */
static const int hole_target_offset_unused = -1;

/**
 * An entrance. Best thought of as a spot <em>just inside</em> a location; that is, "after taking this entrance, Link
 * will be just inside location name."
 */
typedef struct entrance {
    /**
     * The name of the entrance.
     */
    const char *name;

    /**
     * The ROM offset at which to find the entrance's target.
     */
    int target_offset;

    /**
     * The value at target_offset in a vanilla ALttP ROM (JP v1.0).
     *
     * @remark This seems to identify the target <em>tileset</em>, not the specific location.
     */
    uint8_t vanilla_target_value;

    /**
     * The exterior region to which this entrance belongs in a vanilla ALttP ROM (JPv1.0). For example, the vanilla
     * "Desert Palace Entrance (South)" entrance belongs to the REGION_EXT_LW_OVERWORLD exterior region.
     */
    enum region vanilla_exterior_region;

    /**
     * The interior region to which this entrance's target belongs in a vanilla ALttP ROM (JP v1.0). For example, the
     * vanilla "Desert Palace Entrance (South)" entrance belongs to the REGION_INT_LW_DESERT_PALACE_FIRST_PART
     * interior region.
     */
    enum region vanilla_interior_region;
} entrance;

/**
 * An exit. Best thought of as a spot in the overworld <em>just outside</em> a location; that is, "after taking this exit,
 * Link will be just outside location name."
 *
 * @remark This is named interior_exit to discourage naming individual exits exit, which is, unfortunately,
 * the name of a global C function.
 */
typedef struct interior_exit {
    /**
     * The name of the exit.
     */
    const char *name;

    /**
     * The ROM offset at which to find the overworld area in which the exit is located.
     */
    int overworld_area_offset;

    /**
     * The ROM offset at which to find the "VRAM location" (itself an offset to, presumably, the tile data for the exit's
     * location in the overworld) of the exit.
     */
    int target_vram_location_offset;

    /**
     * The value at overworld_area_offset in a vanilla ALttP ROM (JP v1.0).
     */
    uint8_t vanilla_overworld_area_value;

    /**
     * The value at target_vram_location_offset in a vanilla ALttP ROM (JP v1.0).
     *
     * @remark This value serves as a unique identifier for an exit. For example, if we load up a ROM and look at the
     * target_vram_location_offset for the "Ice Palace" exit, and the value we find there matches the
     * vanilla_target_vram_location_value for the "Spectacle Rock Cave (Top)" exit, we know that exiting the Ice
     * Palace will put us right outside of Spectacle Rock Cave (Top).
     */
    uint16_t vanilla_target_vram_location_value;

    /**
     * The exterior region to which this exit's target belongs in a vanilla ALttP ROM (JP v1.0). For example, the
     * vanilla "Misery Mire" exit belongs to the REGION_EXT_DW_MIRE region.
     *
     * @remark Unlike entrances, interior_exits do not have both exterior and interior regions;
     * every single interior_exit has a paired entrance, whose interior region can be used, and
     * the vast majority of interior regions are isolated anyway.
     */
    enum region vanilla_exterior_region;
} interior_exit;

/**
 * A hole. This should be thought of as a spot in the overworld <em>just outside</em> a hole into which Link can fall;
 * that is, "jumping through the hole at name will take Link to the target identified by
 * vanilla_target_value."
 */
typedef struct hole {
    /**
     * The name of the hole.
     */
    const char *name;

    /**
     * The ROM offsets at which to find the hole's target. Some holes have two offsets; if a hole has only one offset,
     * the second will be hole_target_offset_unused.
     *
     * @remark I am unsure why some holes have two offsets.
     */
    int target_offsets[2];

    /**
     * The value at all used offsets in target_offsets in a vanilla ALttP ROM (JP v1.0).
     *
     * @remark This value identifies an entrance, but not uniquely; interior locations that use the same tileset (e.g.,
     * various shops, healer fairy caves, etc.) share the same value here.
     *
     * @todo Figure out how to differentiate between entrances that share this value. I've asked about it here:
     * https://discord.com/channels/731205301247803413/827141303330406408/1490853350927896669 but have received no
     * response.
     */
    uint8_t vanilla_target_value;

    /**
     * The name of the target interior location in a vanilla ALttP ROM (JP v1.0). In general, the target is just the
     * entrance corresponding to the only way out. There are two exceptions to this: the "Sanctuary Grave" hole targets
     * the "Hyrule Castle (Sout)" entrance, and the "Skull Woods Second Section Hole" targets the "Skull Woods Second
     * Section (West)" entrance.
     */
    const char *vanilla_target_name;

    /**
     * The exterior region in which this hole is located in a vanilla ALttP ROM (JP v1.0). For example, the "Kakariko
     * Well Drop" is in the REGION_EXT_LW_OVERWORLD region.
     */
    enum region vanilla_region;
} hole;

/**
 * A unidirectional link between two regions that does not require the use of an entrance, exit, or hole.
 */
typedef struct region_link {
    /**
     * The origin region.
     */
    enum region origin_region;

    /**
     * The destination region.
     */
    enum region destination_region;

    /**
     * The method by which Link moves from origin_region to destination_region.
     */
    const char *method;
} region_link;

/**
 * A link between an entrance and an interior offset. This represents a pair of locations that are just inside/outside
 * the other in a vanilla ALttP ROM (JP v1.0).
 */
typedef struct location_link {
    /**
     * The name of the location.
     */
    const char *name;

    /**
     * The entrance, represented as an offset (index) of entrances.
     */
    size_t entrances_offset;

    /**
     * The exit, represent as an offset (index) of interior_exits.
     */
    size_t interior_exits_offset;
} location_link;

/**
 * All entrances, with data sourced from a vanilla ALttP ROM (JP v1.0).
 */
extern const entrance entrances[];

/**
 * The number of elements in entrances.
 */
extern const size_t num_entrances;

/**
 * All exit locations, with data sourced from a vanilla ALttP ROM (JP v1.0).
 */
extern const interior_exit interior_exits[];

/**
 * The number of elements in interior_exits.
 */
extern const size_t num_interior_exits;

/**
 * All holes, with data sourced from a vanilla ALttP ROM (JP v1.0).
 */
extern const hole holes[];

/**
 * The number of elements in holes.
 */
extern const size_t num_holes;

/**
 * All unidirectional region links.
 */
extern const region_link region_links[];

/**
 * The number of elements in region_links.
 */
extern const size_t num_region_links;

/**
 * All location links.
 *
 * @remark Technically, this list isn't necessary. We could simply string-compare entrance and exit names while
 * building the world, but, of all the properties of locations, the name is the one that we're the most likely to
 * change (likely to add additional descriptors for rare/vague names). It is FAR less likely that the number or
 * order of entrances/exits will change.
 */
extern const location_link location_links[];

/**
 * The number of elements in location_links.
 */
extern const size_t num_location_links;

#endif // ALTTPR_PATHFINDER_LOCATIONS_H
