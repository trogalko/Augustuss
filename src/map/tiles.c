#include "tiles.h"

#include "assets/assets.h"
#include "building/building.h"
#include "city/map.h"
#include "city/view.h"
#include "core/direction.h"
#include "core/image.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/data.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/image_context.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/map.h"

#define OFFSET(x,y) (x + GRID_SIZE * y)

#define FORBIDDEN_TERRAIN_MEADOW (TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |\
            TERRAIN_RUBBLE | TERRAIN_ROAD | TERRAIN_BUILDING | TERRAIN_GARDEN | TERRAIN_WALL)

#define FORBIDDEN_TERRAIN_RUBBLE (TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |\
            TERRAIN_ROAD | TERRAIN_BUILDING | TERRAIN_GARDEN)

#define GARDEN_VARIANTS 2
#define GARDEN_IMAGES_PER_VARIANT 4

static int aqueduct_include_construction = 0;
static int highway_top_tile_offsets[4] = { 0, -GRID_SIZE, -1, -GRID_SIZE - 1 };
static int elevation_recalculate_trees = 0;

static int is_clear(int x, int y, int size, int disallowed_terrain, int check_figure, int check_image)
{
    if (!map_grid_is_inside(x, y, size)) {
        return 0;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR & disallowed_terrain)) {
                return 0;
            } else if (check_figure && map_has_figure_at(grid_offset)) {
                return 0;
            } else if (check_image && map_image_at(grid_offset)) {
                return 0;
            }
        }
    }
    return 1;
}

int map_tiles_are_clear(int x, int y, int size, int disallowed_terrain, int check_figure)
{
    return is_clear(x, y, size, disallowed_terrain, check_figure, 0);
}

static void foreach_map_tile(void (*callback)(int x, int y, int grid_offset))
{
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            callback(x, y, grid_offset);
        }
    }
}

static void foreach_region_tile(int x_min, int y_min, int x_max, int y_max,
    void (*callback)(int x, int y, int grid_offset))
{
    map_grid_bound_area(&x_min, &y_min, &x_max, &y_max);
    int grid_offset = map_grid_offset(x_min, y_min);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            callback(xx, yy, grid_offset);
            ++grid_offset;
        }
        grid_offset += GRID_SIZE - (x_max - x_min + 1);
    }
}

static int is_all_terrain_in_area(int x, int y, int size, int terrain)
{
    if (!map_grid_is_inside(x, y, size)) {
        return 0;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            if ((map_terrain_get(grid_offset) & TERRAIN_NOT_CLEAR) != terrain) {
                return 0;
            }
            if (map_image_at(grid_offset) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

static int is_updatable_rock(int grid_offset)
{
    return map_terrain_is(grid_offset, TERRAIN_ROCK) &&
        !map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP);
}

static void clear_rock_image(int x, int y, int grid_offset)
{
    if (is_updatable_rock(grid_offset)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static void set_rock_image(int x, int y, int grid_offset)
{
    if (is_updatable_rock(grid_offset)) {
        if (!map_image_at(grid_offset)) {
            if (is_all_terrain_in_area(x, y, 3, TERRAIN_ROCK)) {
                int image_id = 12 + (map_random_get(grid_offset) & 1);
                if (map_terrain_exists_tile_in_radius_with_type(x, y, 3, 4, TERRAIN_ELEVATION)) {
                    image_id += image_group(GROUP_TERRAIN_ELEVATION_ROCK);
                } else {
                    image_id += image_group(GROUP_TERRAIN_ROCK);
                }
                map_building_tiles_add(0, x, y, 3, image_id, TERRAIN_ROCK);
            } else if (is_all_terrain_in_area(x, y, 2, TERRAIN_ROCK)) {
                int image_id = 8 + (map_random_get(grid_offset) & 3);
                if (map_terrain_exists_tile_in_radius_with_type(x, y, 2, 4, TERRAIN_ELEVATION)) {
                    image_id += image_group(GROUP_TERRAIN_ELEVATION_ROCK);
                } else {
                    image_id += image_group(GROUP_TERRAIN_ROCK);
                }
                map_building_tiles_add(0, x, y, 2, image_id, TERRAIN_ROCK);
            } else {
                int image_id = map_random_get(grid_offset) & 7;
                if (map_terrain_exists_tile_in_radius_with_type(x, y, 1, 4, TERRAIN_ELEVATION)) {
                    image_id += image_group(GROUP_TERRAIN_ELEVATION_ROCK);
                } else {
                    image_id += image_group(GROUP_TERRAIN_ROCK);
                }
                map_image_set(grid_offset, image_id);
            }
        }
    }
}

void map_tiles_update_all_rocks(void)
{
    foreach_map_tile(clear_rock_image);
    foreach_map_tile(set_rock_image);
}

static void update_tree_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_TREE) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        int image_id = image_group(GROUP_TERRAIN_TREE) + (map_random_get(grid_offset) & 7);
        if (map_terrain_has_only_rocks_trees_in_ring(x, y, 3)) {
            map_image_set(grid_offset, image_id + 24);
        } else if (map_terrain_has_only_rocks_trees_in_ring(x, y, 2)) {
            map_image_set(grid_offset, image_id + 16);
        } else if (map_terrain_has_only_rocks_trees_in_ring(x, y, 1)) {
            map_image_set(grid_offset, image_id + 8);
        } else {
            map_image_set(grid_offset, image_id);
        }
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
        map_aqueduct_remove(grid_offset);
    }
}

static void set_tree_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_TREE) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        foreach_region_tile(x - 1, y - 1, x + 1, y + 1, update_tree_image);
    }
}

void map_tiles_update_region_trees(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, set_tree_image);
}

static void set_shrub_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_SHRUB) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        map_image_set(grid_offset, image_group(GROUP_TERRAIN_SHRUB) + (map_random_get(grid_offset) & 7));
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

void map_tiles_update_region_shrub(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, set_shrub_image);
}

static void clear_garden_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_GARDEN) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
    if (!map_terrain_is(grid_offset, TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_ROCK)) {
        map_property_clear_plaza_earthquake_or_overgrown_garden(grid_offset);
    }
}

static int is_large_garden(int x, int y, int is_overgrown_garden)
{
    if (!map_grid_is_inside(x, y, 2)) {
        return 0;
    }
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            if ((map_terrain_get(grid_offset) & TERRAIN_NOT_CLEAR) != TERRAIN_GARDEN) {
                return 0;
            }
            int grid_is_overgrown_garden = map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset) != 0;
            if (grid_is_overgrown_garden != is_overgrown_garden) {
                return 0;
            }
            if (map_image_at(grid_offset) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

static void set_garden_image(int x, int y, int grid_offset)
{
    static int garden_image_ids[GARDEN_VARIANTS][GARDEN_IMAGES_PER_VARIANT];
    if (!garden_image_ids[0][1]) {
        garden_image_ids[0][0] = assets_get_image_id("Aesthetics", "Garden_Alt_01");
        garden_image_ids[0][1] = image_group(GROUP_TERRAIN_GARDEN) + 1;
        garden_image_ids[0][2] = garden_image_ids[0][1] + 1;
        garden_image_ids[0][3] = garden_image_ids[0][0] + 1;
        
        garden_image_ids[1][0] = assets_get_image_id("Aesthetics", "Overgrown_Garden_01");
        garden_image_ids[1][1] = garden_image_ids[1][0] + 1;
        garden_image_ids[1][2] = garden_image_ids[1][0] + 2;
        garden_image_ids[1][3] = garden_image_ids[1][0] + 3;
    }

    if (map_terrain_is(grid_offset, TERRAIN_GARDEN) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        if (!map_image_at(grid_offset)) {
            int is_overgrown_garden = map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset) != 0;
            int image_id = is_overgrown_garden ? garden_image_ids[1][0] : image_group(GROUP_TERRAIN_GARDEN);
            if (is_large_garden(x, y, is_overgrown_garden)) {
                if (!is_overgrown_garden) {
                    switch (map_random_get(grid_offset) & 3) {
                        case 0: case 1:
                            image_id += 6;
                            break;
                        case 2:
                            image_id += 5;
                            break;
                        case 3:
                            image_id += 4;
                            break;
                    }
                } else {
                    switch (map_random_get(grid_offset) & 3) {
                        case 0: case 1:
                            image_id += 5;
                            break;
                        case 2:
                            image_id += 4;
                            break;
                        case 3:
                            image_id += 6;
                            break;
                    }
                }
                map_building_tiles_add(0, x, y, 2, image_id, TERRAIN_GARDEN);
            } else {
                image_id = garden_image_ids[is_overgrown_garden][(x + y) % GARDEN_IMAGES_PER_VARIANT];
            }
            map_image_set(grid_offset, image_id);
        }
    }
}

static void remove_plaza_below_building(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
        if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            map_property_clear_plaza_earthquake_or_overgrown_garden(grid_offset);
        }
    }
}

static void clear_plaza_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static int is_tile_plaza(int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset) &&
        !map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING) &&
        !map_image_at(grid_offset)) {
        return 1;
    }
    return 0;
}

static int is_two_tile_square_plaza(int grid_offset)
{
    return
        is_tile_plaza(grid_offset + map_grid_delta(1, 0)) &&
        is_tile_plaza(grid_offset + map_grid_delta(0, 1)) &&
        is_tile_plaza(grid_offset + map_grid_delta(1, 1));
}

static void set_plaza_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset) &&
        !map_image_at(grid_offset)) {
        if (is_two_tile_square_plaza(grid_offset)) {
            int image_id = image_group(GROUP_TERRAIN_PLAZA);
            if (map_random_get(grid_offset) & 1) {
                image_id += 7;
            } else {
                image_id += 6;
            }
            map_building_tiles_add(0, x, y, 2, image_id, TERRAIN_ROAD);
        } else {
            // single tile plaza
            static int image_id;
            if (!image_id) {
                image_id = assets_get_image_id("Aesthetics", "Plazas");
            }
            int image_offset = (x + y) % 9;
            map_image_set(grid_offset, image_id + image_offset);
        }
    }
}

void map_tiles_update_all_gardens(void)
{
    foreach_map_tile(clear_garden_image);
    foreach_map_tile(set_garden_image);
}

void map_tiles_update_all_plazas(void)
{
    foreach_map_tile(remove_plaza_below_building);
    foreach_map_tile(clear_plaza_image);
    foreach_map_tile(set_plaza_image);
}

static int get_gatehouse_building_id(int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_GATEHOUSE)) {
        return map_building_at(grid_offset);
    }
    return 0;
}

static int get_gatehouse_position(int grid_offset, int direction, int building_id)
{
    int result = 0;
    if (direction == DIR_0_TOP) {
        if (map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(1, -1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(-1, -1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    } else if (direction == DIR_6_LEFT) {
        if (map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(-1, 1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(-1, -1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    } else if (direction == DIR_4_BOTTOM) {
        if (map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(1, 1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(-1, 1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    } else if (direction == DIR_2_RIGHT) {
        if (map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(1, 1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_GATEHOUSE) &&
            map_building_at(grid_offset + map_grid_delta(1, -1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    }
    return result;
}

static void set_wall_gatehouse_image_manually(int grid_offset)
{
    int gatehouse_up = get_gatehouse_building_id(grid_offset + map_grid_delta(0, -1));
    int gatehouse_left = get_gatehouse_building_id(grid_offset + map_grid_delta(-1, 0));
    int gatehouse_down = get_gatehouse_building_id(grid_offset + map_grid_delta(0, 1));
    int gatehouse_right = get_gatehouse_building_id(grid_offset + map_grid_delta(1, 0));
    int image_offset = 0;
    int map_orientation = city_view_orientation();
    if (map_orientation == DIR_0_TOP) {
        if (gatehouse_up && !gatehouse_left) {
            int pos = get_gatehouse_position(grid_offset, DIR_0_TOP, gatehouse_up);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 29;
                } else if (pos == 3) {
                    image_offset = 31;
                } else {
                    image_offset = 33;
                }
            }
        } else if (gatehouse_left && !gatehouse_up) {
            int pos = get_gatehouse_position(grid_offset, DIR_6_LEFT, gatehouse_left);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 30;
                } else if (pos == 3) {
                    image_offset = 32;
                } else {
                    image_offset = 33;
                }
            }
        }
    } else if (map_orientation == DIR_2_RIGHT) {
        if (gatehouse_up && !gatehouse_right) {
            int pos = get_gatehouse_position(grid_offset, DIR_0_TOP, gatehouse_up);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 32;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 30;
                }
            }
        } else if (gatehouse_right && !gatehouse_up) {
            int pos = get_gatehouse_position(grid_offset, DIR_2_RIGHT, gatehouse_right);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 29;
                } else if (pos == 3) {
                    image_offset = 31;
                } else {
                    image_offset = 33;
                }
            }
        }
    } else if (map_orientation == DIR_4_BOTTOM) {
        if (gatehouse_down && !gatehouse_right) {
            int pos = get_gatehouse_position(grid_offset, DIR_4_BOTTOM, gatehouse_down);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 31;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 29;
                }
            }
        } else if (gatehouse_right && !gatehouse_down) {
            int pos = get_gatehouse_position(grid_offset, DIR_2_RIGHT, gatehouse_right);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 32;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 30;
                }
            }
        }
    } else if (map_orientation == DIR_6_LEFT) {
        if (gatehouse_down && !gatehouse_left) {
            int pos = get_gatehouse_position(grid_offset, DIR_4_BOTTOM, gatehouse_down);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 30;
                } else if (pos == 3) {
                    image_offset = 32;
                } else {
                    image_offset = 33;
                }
            }
        } else if (gatehouse_left && !gatehouse_down) {
            int pos = get_gatehouse_position(grid_offset, DIR_6_LEFT, gatehouse_left);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 31;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 29;
                }
            }
        }
    }
    if (image_offset) {
        map_image_set(grid_offset, image_group(GROUP_BUILDING_WALL) + image_offset);
    }
}

static void set_wall_image(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_WALL) ||
        map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return;
    }
    const terrain_image *img = map_image_context_get_wall(grid_offset);
    map_image_set(grid_offset, image_group(GROUP_BUILDING_WALL) +
        img->group_offset + img->item_offset);
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
    if (map_terrain_count_directly_adjacent_with_type(grid_offset, TERRAIN_GATEHOUSE) > 0) {
        img = map_image_context_get_wall_gatehouse(grid_offset);
        if (img->is_valid) {
            map_image_set(grid_offset, image_group(GROUP_BUILDING_WALL) +
                img->group_offset + img->item_offset);
        } else {
            set_wall_gatehouse_image_manually(grid_offset);
        }
    }
}

void map_tiles_update_all_walls(void)
{
    foreach_map_tile(set_wall_image);
}

void map_tiles_update_area_walls(int x, int y, int size)
{
    foreach_region_tile(x - 1, y - 1, x + size - 2, y + size - 2, set_wall_image);
}

int map_tiles_set_wall(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    int tile_set = 0;
    if (!map_terrain_is(grid_offset, TERRAIN_WALL)) {
        tile_set = 1;
    }
    map_terrain_add(grid_offset, TERRAIN_WALL);
    map_property_clear_constructing(grid_offset);

    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_wall_image);
    return tile_set;
}

int map_tiles_is_paved_road(int grid_offset)
{
    int desirability = map_desirability_get(grid_offset);
    if (desirability > 4) {
        return 1;
    }
    if (desirability > 0 && map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
        return 1;
    }
    int x = map_grid_offset_to_x(grid_offset);
    int y = map_grid_offset_to_y(grid_offset);
    if (map_terrain_exists_tile_in_radius_with_type(x, y, 1, 3, TERRAIN_HIGHWAY)) {
        return 1;
    }
    return 0;
}

int map_tiles_highway_get_aqueduct_image(int grid_offset)
{
    int aqueduct_image_id = assets_lookup_image_id(ASSET_AQUEDUCT_WITH_WATER);
    int image_offset = 0;
    if (map_terrain_is(grid_offset - 1, TERRAIN_AQUEDUCT) || map_terrain_is(grid_offset + 1, TERRAIN_AQUEDUCT)) {
        image_offset++;
    }
    if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
        image_offset = (image_offset + 1) % 2;
    }
    if (!map_aqueduct_has_water_access_at(grid_offset)) {
        image_offset += 2;
    }
    return aqueduct_image_id + image_offset;
}

static void set_aqueduct_image(int grid_offset, int is_road, const terrain_image *img)
{
    int new_image_id = 0;
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
        new_image_id = map_tiles_highway_get_aqueduct_image(grid_offset);
    } else {
        int group_offset = img->group_offset;
        if (is_road) {
            if (!img->aqueduct_offset || (group_offset != 2 && group_offset != 3)) {
                if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
                    group_offset = 3;
                } else {
                    group_offset = 2;
                }
            }
            if (map_tiles_is_paved_road(grid_offset)) {
                group_offset -= 2;
            } else {
                group_offset += 6;
            }
        }
        int image_aqueduct = image_group(GROUP_BUILDING_AQUEDUCT);
        int water_offset;
        int image_id = map_image_at(grid_offset);
        if (image_id >= image_aqueduct && image_id < image_aqueduct + 15) {
            water_offset = 0;
        } else {
            water_offset = 15;
        }
        new_image_id = image_aqueduct + water_offset + group_offset;
    }
    map_image_set(grid_offset, new_image_id);
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
}

static void set_road_with_aqueduct_image(int grid_offset)
{
    set_aqueduct_image(grid_offset, 1, map_image_context_get_aqueduct(grid_offset, 0));
}

static void set_road_image(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_ROAD) ||
        map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING)) {
        return;
    }
    if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        set_road_with_aqueduct_image(grid_offset);
        return;
    }
    if (map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
        return;
    }
    if (map_tiles_is_paved_road(grid_offset)) {
        const terrain_image *img = map_image_context_get_paved_road(grid_offset);
        map_image_set(grid_offset, image_group(GROUP_TERRAIN_ROAD) +
            img->group_offset + img->item_offset);
    } else {
        const terrain_image *img = map_image_context_get_dirt_road(grid_offset);
        map_image_set(grid_offset, image_group(GROUP_TERRAIN_ROAD) +
            img->group_offset + img->item_offset + 49);
    }
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
}

static void set_highway_image(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_HIGHWAY) || map_terrain_is(grid_offset, TERRAIN_GATEHOUSE)) {
        return;
    }
    if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        const terrain_image *img = map_image_context_get_aqueduct(grid_offset, 0);
        set_aqueduct_image(grid_offset, 0, img);
    } else {
        int highway_base = assets_lookup_image_id(ASSET_HIGHWAY_BASE_START);
        map_image_set(grid_offset, highway_base);
    }
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
}

void map_tiles_update_all_roads(void)
{
    foreach_map_tile(set_road_image);
}

void map_tiles_update_area_roads(int x, int y, int size)
{
    foreach_region_tile(x - 1, y - 1, x + size - 2, y + size - 2, set_road_image);
}

int map_tiles_set_road(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
        return 0;
    }
    int tile_set = 0;
    if (!map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        tile_set = 1;
    }
    map_terrain_add(grid_offset, TERRAIN_ROAD);
    map_property_clear_constructing(grid_offset);

    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_road_image);
    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_highway_image);
    return tile_set;
}

void map_tiles_update_all_highways(void)
{
    foreach_map_tile(set_highway_image);
}

void map_tiles_update_area_highways(int x, int y, int size)
{
    foreach_region_tile(x - 1, y - 1, x + size, y + size, set_highway_image);
}

int map_tiles_set_highway(int x, int y)
{
    int items = 0;
    int terrain = TERRAIN_HIGHWAY_TOP_LEFT;
    for (int xx = x; xx <= x + 1; xx++) {
        for (int yy = y; yy <= y + 1; yy++) {
            int grid_offset = map_grid_offset(xx, yy);
            if (!map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
                items++;
            }
            map_terrain_remove(grid_offset, TERRAIN_ROAD);
            map_terrain_add(grid_offset, terrain);
            map_property_clear_constructing(grid_offset);
            terrain <<= 1;
        }
    }
    foreach_region_tile(x - 1, y - 1, x + 2, y + 2, set_highway_image);
    foreach_region_tile(x - 1, y - 1, x + 2, y + 2, set_road_image);
    return items;
}

static int clear_highway_from_top(int grid_offset, int measure_only)
{
    int cleared = 0;
    int x = map_grid_offset_to_x(grid_offset);
    int y = map_grid_offset_to_y(grid_offset);
    int terrain = TERRAIN_HIGHWAY_TOP_LEFT;
    for (int xx = x; xx <= x + 1; xx++) {
        for (int yy = y; yy <= y + 1; yy++) {
            int highway_offset = map_grid_offset(xx, yy);
            if (!map_terrain_is(highway_offset, TERRAIN_HIGHWAY)) {
                continue;
            }
            map_property_mark_deleted(highway_offset);
            if (!measure_only) {
                map_terrain_remove(highway_offset, terrain);
            }
            terrain <<= 1;
            cleared = 1;
        }
    }
    foreach_region_tile(x - 1, y - 1, x + 2, y + 2, set_highway_image);
    return cleared;
}

int map_tiles_clear_highway(int grid_offset, int measure_only)
{
    int items_cleared = 0;
    int terrain = map_terrain_get(grid_offset);
    int highway_terrain = TERRAIN_HIGHWAY_TOP_LEFT;
    for (int i = 0; i < 4; i++) {
        if (terrain & highway_terrain) {
            int highway_top_tile = grid_offset + highway_top_tile_offsets[i];
            items_cleared += clear_highway_from_top(highway_top_tile, measure_only);
        }
        highway_terrain <<= 1;
    }
    return items_cleared;
}

static void clear_empty_land_image(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static void set_empty_land_image(int x, int y, int size, int image_id)
{
    if (!map_grid_is_inside(x, y, size)) {
        return;
    }
    int index = 0;
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
            map_building_set(grid_offset, 0);
            map_property_clear_constructing(grid_offset);
            map_property_set_multi_tile_size(grid_offset, 1);
            map_property_mark_draw_tile(grid_offset);
            map_image_set(grid_offset, image_id + index);
            index++;
        }
    }
}

static void set_empty_land_pass1(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR) && !map_image_at(grid_offset) &&
        !(map_random_get(grid_offset) & 0xf0)) {
        int image_id;
        if (map_property_is_alternate_terrain(grid_offset)) {
            image_id = image_group(GROUP_TERRAIN_GRASS_2);
        } else {
            image_id = image_group(GROUP_TERRAIN_GRASS_1);
        }
        set_empty_land_image(x, y, 1, image_id + (map_random_get(grid_offset) & 7));
    }
}

static void set_empty_land_pass2(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR) && !map_image_at(grid_offset)) {
        int image_id;
        if (map_property_is_alternate_terrain(grid_offset)) {
            image_id = image_group(GROUP_TERRAIN_GRASS_2);
        } else {
            image_id = image_group(GROUP_TERRAIN_GRASS_1);
        }
        if (is_clear(x, y, 4, TERRAIN_ALL, 1, 1)) {
            set_empty_land_image(x, y, 4, image_id + 42);
        } else if (is_clear(x, y, 3, TERRAIN_ALL, 1, 1)) {
            set_empty_land_image(x, y, 3, image_id + 24 + 9 * (map_random_get(grid_offset) & 1));
        } else if (is_clear(x, y, 2, TERRAIN_ALL, 1, 1)) {
            set_empty_land_image(x, y, 2, image_id + 8 + 4 * (map_random_get(grid_offset) & 3));
        } else {
            set_empty_land_image(x, y, 1, image_id + (map_random_get(grid_offset) & 7));
        }
    }
}

void map_tiles_update_all_empty_land(void)
{
    foreach_map_tile(clear_empty_land_image);
    foreach_map_tile(set_empty_land_pass1);
    foreach_map_tile(set_empty_land_pass2);
}

void map_tiles_update_region_empty_land(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, clear_empty_land_image);
    foreach_region_tile(x_min, y_min, x_max, y_max, set_empty_land_pass1);
    foreach_region_tile(x_min, y_min, x_max, y_max, set_empty_land_pass2);
}

static void set_meadow_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_MEADOW) && !map_terrain_is(grid_offset, FORBIDDEN_TERRAIN_MEADOW)) {
        int random = map_random_get(grid_offset) & 3;
        int image_id = image_group(GROUP_TERRAIN_MEADOW);
        if (map_terrain_has_only_meadow_in_ring(x, y, 2)) {
            map_image_set(grid_offset, image_id + random + 8);
        } else if (map_terrain_has_only_meadow_in_ring(x, y, 1)) {
            map_image_set(grid_offset, image_id + random + 4);
        } else {
            map_image_set(grid_offset, image_id + random);
        }
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
        map_aqueduct_remove(grid_offset);
    }
}

static void update_meadow_tile(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_MEADOW) && !map_terrain_is(grid_offset, FORBIDDEN_TERRAIN_MEADOW)) {
        foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_meadow_image);
    }
}

void map_tiles_update_all_meadow(void)
{
    foreach_map_tile(update_meadow_tile);
}

void map_tiles_update_region_meadow(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, update_meadow_tile);
}

static void set_water_image(int x, int y, int grid_offset)
{
    if ((map_terrain_get(grid_offset) & (TERRAIN_WATER | TERRAIN_BUILDING)) == TERRAIN_WATER) {
        const terrain_image *img = map_image_context_get_shore(grid_offset);
        int image_id = image_group(GROUP_TERRAIN_WATER) + img->group_offset + img->item_offset;
        if (map_terrain_exists_tile_in_radius_with_type(x, y, 1, 2, TERRAIN_BUILDING)) {
            // fortified shore
            int base = image_group(GROUP_TERRAIN_WATER_SHORE);
            switch (img->group_offset) {
                case 8: image_id = base + 10; break;
                case 12: image_id = base + 11; break;
                case 16: image_id = base + 9; break;
                case 20: image_id = base + 8; break;
                case 24: image_id = base + 18; break;
                case 28: image_id = base + 16; break;
                case 32: image_id = base + 19; break;
                case 36: image_id = base + 17; break;
                case 50: image_id = base + 12; break;
                case 51: image_id = base + 14; break;
                case 52: image_id = base + 13; break;
                case 53: image_id = base + 15; break;
            }
        }
        map_image_set(grid_offset, image_id);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static void update_water_tile(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_WATER) && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_water_image);
    }
}

void map_tiles_update_all_water(void)
{
    foreach_map_tile(update_water_tile);
}

void map_tiles_update_region_water(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, update_water_tile);
}

void map_tiles_set_water(int x, int y)
{
    map_terrain_add(map_grid_offset(x, y), TERRAIN_WATER);
    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_water_image);
}

static void set_aqueduct(int grid_offset)
{
    const terrain_image *img = map_image_context_get_aqueduct(grid_offset, aqueduct_include_construction);
    int is_road = map_terrain_is(grid_offset, TERRAIN_ROAD | TERRAIN_HIGHWAY);
    if (is_road) {
        map_property_clear_plaza_earthquake_or_overgrown_garden(grid_offset);
    }
    set_aqueduct_image(grid_offset, is_road, img);
    map_aqueduct_set_image(grid_offset, img->aqueduct_offset);
}

static void update_aqueduct_tile(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT) && map_aqueduct_image_at(grid_offset) <= 15) {
        set_aqueduct(grid_offset);
    }
}

void map_tiles_update_all_aqueducts(int include_construction)
{
    aqueduct_include_construction = include_construction;
    foreach_map_tile(update_aqueduct_tile);
    aqueduct_include_construction = 0;
}

void map_tiles_update_region_aqueducts(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, update_aqueduct_tile);
}

static void set_earthquake_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROCK) &&
        map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
        const terrain_image *img = map_image_context_get_earthquake(grid_offset);
        if (img->is_valid) {
            map_image_set(grid_offset,
                image_group(GROUP_TERRAIN_EARTHQUAKE) + img->group_offset + img->item_offset);
        } else {
            map_image_set(grid_offset, image_group(GROUP_TERRAIN_EARTHQUAKE));
        }
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static void update_earthquake_tile(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROCK) &&
        map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
        map_terrain_add(grid_offset, TERRAIN_ROCK);
        map_property_mark_plaza_earthquake_or_overgrown_garden(grid_offset);
        foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_earthquake_image);
    }
}

void map_tiles_update_all_earthquake(void)
{
    foreach_map_tile(update_earthquake_tile);
}

void map_tiles_set_earthquake(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    // earthquake: terrain = rock && bitfields = plaza
    map_terrain_add(grid_offset, TERRAIN_ROCK);
    map_property_mark_plaza_earthquake_or_overgrown_garden(grid_offset);

    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_earthquake_image);
}

static void set_rubble_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_RUBBLE) && !map_terrain_is(grid_offset, FORBIDDEN_TERRAIN_RUBBLE)) {
        map_image_set(grid_offset, image_group(GROUP_TERRAIN_RUBBLE) + (map_random_get(grid_offset) & 7));
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
        map_aqueduct_remove(grid_offset);
    }
}

void map_tiles_update_all_rubble(void)
{
    foreach_map_tile(set_rubble_image);
}

void map_tiles_update_region_rubble(int x_min, int y_min, int x_max, int y_max)
{
    foreach_region_tile(x_min, y_min, x_max, y_max, set_rubble_image);
}

static void clear_access_ramp_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ACCESS_RAMP)) {
        map_image_set(grid_offset, 0);
    }
}

static int get_access_ramp_image_offset(int x, int y)
{
    if (!map_grid_is_inside(x, y, 1)) {
        return -1;
    }
    static const int offsets[4][6] = {
        {OFFSET(0,1), OFFSET(1,1), OFFSET(0,0), OFFSET(1,0), OFFSET(0,2), OFFSET(1,2)},
        {OFFSET(0,0), OFFSET(0,1), OFFSET(1,0), OFFSET(1,1), OFFSET(-1,0), OFFSET(-1,1)},
        {OFFSET(0,0), OFFSET(1,0), OFFSET(0,1), OFFSET(1,1), OFFSET(0,-1), OFFSET(1,-1)},
        {OFFSET(1,0), OFFSET(1,1), OFFSET(0,0), OFFSET(0,1), OFFSET(2,0), OFFSET(2,1)},
    };
    int base_offset = map_grid_offset(x, y);
    int image_offset = -1;
    for (int dir = 0; dir < 4; dir++) {
        int right_tiles = 0;
        int height = -1;
        for (int i = 0; i < 6; i++) {
            int grid_offset = base_offset + offsets[dir][i];
            if (i < 2) { // 2nd row
                if (map_terrain_is(grid_offset, TERRAIN_ELEVATION)) {
                    right_tiles++;
                }
                height = map_elevation_at(grid_offset);
            } else if (i < 4) { // 1st row
                if (map_terrain_is(grid_offset, TERRAIN_ACCESS_RAMP) &&
                    map_elevation_at(grid_offset) < height) {
                    right_tiles++;
                }
            } else { // higher row beyond access ramp
                if (map_terrain_is(grid_offset, TERRAIN_ELEVATION)) {
                    if (map_elevation_at(grid_offset) != height) {
                        right_tiles++;
                    }
                } else if (map_elevation_at(grid_offset) >= height) {
                    right_tiles++;
                }
            }
        }
        if (right_tiles == 6) {
            image_offset = dir;
            break;
        }
    }
    if (image_offset < 0) {
        return -1;
    }
    switch (city_view_orientation()) {
        case DIR_0_TOP: break;
        case DIR_6_LEFT: image_offset += 1; break;
        case DIR_4_BOTTOM: image_offset += 2; break;
        case DIR_2_RIGHT: image_offset += 3; break;
    }
    if (image_offset >= 4) {
        image_offset -= 4;
    }
    return image_offset;
}

static void set_elevation_aqueduct_image(int grid_offset)
{
    if (map_aqueduct_image_at(grid_offset) <= 15 && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        set_aqueduct(grid_offset);
    }
}

static void set_elevation_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ACCESS_RAMP) && !map_image_at(grid_offset)) {
        int image_offset = get_access_ramp_image_offset(x, y);
        if (image_offset < 0) {
            // invalid map: remove access ramp
            map_terrain_remove(grid_offset, TERRAIN_ACCESS_RAMP);
            map_property_set_multi_tile_size(grid_offset, 1);
            map_property_mark_draw_tile(grid_offset);
            if (map_elevation_at(grid_offset)) {
                map_terrain_add(grid_offset, TERRAIN_ELEVATION);
            } else {
                map_terrain_remove(grid_offset, TERRAIN_ELEVATION);
                map_image_set(grid_offset,
                    image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7));
            }
        } else {
            map_building_tiles_add(0, x, y, 2,
                image_group(GROUP_TERRAIN_ACCESS_RAMP) + image_offset, TERRAIN_ACCESS_RAMP);
        }
    }
    if (map_elevation_at(grid_offset) && !map_terrain_is(grid_offset, TERRAIN_ACCESS_RAMP)) {
        const terrain_image *img = map_image_context_get_elevation(grid_offset, map_elevation_at(grid_offset));
        if (img->group_offset == 44) {
            map_terrain_remove(grid_offset, TERRAIN_ELEVATION);
            int terrain = map_terrain_get(grid_offset);
            if (!(terrain & TERRAIN_BUILDING)) {
                map_property_set_multi_tile_xy(grid_offset, 0, 0, 1);
                if (terrain & TERRAIN_SHRUB) {
                    map_image_set(grid_offset, image_group(GROUP_TERRAIN_SHRUB) + (map_random_get(grid_offset) & 7));
                } else if (terrain & TERRAIN_TREE) {
                    if (elevation_recalculate_trees) {
                        update_tree_image(x, y, grid_offset);
                    }
                } else if (terrain & TERRAIN_ROAD) {
                    map_tiles_set_road(x, y);
                } else if (terrain & TERRAIN_AQUEDUCT) {
                    set_elevation_aqueduct_image(grid_offset);
                } else if (terrain & TERRAIN_MEADOW) {
                    map_image_set(grid_offset, image_group(GROUP_TERRAIN_MEADOW) + (map_random_get(grid_offset) & 3));
                } else {
                    map_image_set(grid_offset, image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7));
                }
            }
        } else {
            map_property_set_multi_tile_xy(grid_offset, 0, 0, 1);
            map_terrain_add(grid_offset, TERRAIN_ELEVATION);
            map_image_set(grid_offset, image_group(GROUP_TERRAIN_ELEVATION) + img->group_offset + img->item_offset);
        }
    }
}

static void update_all_elevation(int recalculate_trees)
{
    elevation_recalculate_trees = recalculate_trees;
    int width = map_data.width - 1;
    int height = map_data.height - 1;
    foreach_region_tile(0, 0, width, height, clear_access_ramp_image);
    foreach_region_tile(0, 0, width, height, set_elevation_image);
}

void map_tiles_update_all_elevation(void)
{
    update_all_elevation(0);
}

void map_tiles_update_all_elevation_editor(void)
{
    update_all_elevation(1);
}

static void remove_entry_exit_flag(const map_tile *tile)
{
    // re-calculate grid_offset because the stored offset might be invalid
    map_terrain_remove(map_grid_offset(tile->x, tile->y), TERRAIN_ROCK);
}

void map_tiles_remove_entry_exit_flags(void)
{
    remove_entry_exit_flag(city_map_entry_flag());
    remove_entry_exit_flag(city_map_exit_flag());
}

void map_tiles_add_entry_exit_flags(void)
{
    int entry_orientation;
    map_point entry_point = scenario_map_entry();
    if (entry_point.x == 0) {
        entry_orientation = DIR_2_RIGHT;
    } else if (entry_point.x == map_data.width - 1) {
        entry_orientation = DIR_6_LEFT;
    } else if (entry_point.y == 0) {
        entry_orientation = DIR_0_TOP;
    } else if (entry_point.y == map_data.height - 1) {
        entry_orientation = DIR_4_BOTTOM;
    } else {
        entry_orientation = -1;
    }
    int exit_orientation;
    map_point exit_point = scenario_map_exit();
    if (exit_point.x == 0) {
        exit_orientation = DIR_2_RIGHT;
    } else if (exit_point.x == map_data.width - 1) {
        exit_orientation = DIR_6_LEFT;
    } else if (exit_point.y == 0) {
        exit_orientation = DIR_0_TOP;
    } else if (exit_point.y == map_data.height - 1) {
        exit_orientation = DIR_4_BOTTOM;
    } else {
        exit_orientation = -1;
    }
    if (entry_orientation >= 0) {

        int grid_offset_flag = map_grid_offset(city_map_entry_flag()->x, city_map_entry_flag()->y);
        int flag_is_set = city_map_entry_flag()->x || city_map_entry_flag()->y || city_map_entry_flag()->grid_offset;

        if (!flag_is_set || !map_grid_is_valid_offset(grid_offset_flag)) {
            int grid_offset = map_grid_offset(entry_point.x, entry_point.y);
            int x_tile, y_tile;
            for (int i = 1; i < 10; i++) {
                if (map_terrain_exists_clear_tile_in_radius(entry_point.x, entry_point.y,
                    1, i, grid_offset, &x_tile, &y_tile)) {
                    break;
                }
            }
            grid_offset_flag = city_map_set_entry_flag(x_tile, y_tile);
        }
        map_terrain_remove(grid_offset_flag, TERRAIN_MEADOW);
        map_terrain_add(grid_offset_flag, TERRAIN_ROCK);
        int orientation = (city_view_orientation() + entry_orientation) % 8;
        map_image_set(grid_offset_flag, image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS) + orientation / 2);
    }
    if (exit_orientation >= 0) {

        int grid_offset_flag = map_grid_offset(city_map_exit_flag()->x, city_map_exit_flag()->y) ;
        int flag_is_set = city_map_exit_flag()->x || city_map_exit_flag()->y || city_map_exit_flag()->grid_offset;


        if (!flag_is_set || !map_grid_is_valid_offset(grid_offset_flag)) {
            int grid_offset = map_grid_offset(exit_point.x, exit_point.y);
            int x_tile, y_tile;
            for (int i = 1; i < 10; i++) {
                if (map_terrain_exists_clear_tile_in_radius(exit_point.x, exit_point.y,
                    1, i, grid_offset, &x_tile, &y_tile)) {
                    break;
                }
            }
            grid_offset_flag = city_map_set_exit_flag(x_tile, y_tile);
        }
        map_terrain_remove(grid_offset_flag, TERRAIN_MEADOW);
        map_terrain_add(grid_offset_flag, TERRAIN_ROCK);
        int orientation = (city_view_orientation() + exit_orientation) % 8;
        map_image_set(grid_offset_flag, image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS) + 4 + orientation / 2);
    }
}

void map_tiles_update_all(void)
{
    map_tiles_remove_entry_exit_flags();

    map_tiles_update_all_elevation();
    map_tiles_update_all_water();
    map_tiles_update_all_earthquake();
    map_tiles_update_all_rocks();
    foreach_map_tile(set_tree_image);
    foreach_map_tile(set_shrub_image);
    map_tiles_update_all_gardens();

    map_tiles_add_entry_exit_flags();

    map_tiles_update_all_empty_land();
    map_tiles_update_all_meadow();
    map_tiles_update_all_rubble();
    map_tiles_update_all_roads();
    map_tiles_update_all_highways();
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);
}
