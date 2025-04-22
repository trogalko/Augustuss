#include "city_overlay_other.h"

#include "building/animation.h"
#include "building/building.h"
#include "building/industry.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/roadblock.h"
#include "building/rotation.h"
#include "building/storage.h"
#include "city/constants.h"
#include "city/finance.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/string.h"
#include "game/resource.h"
#include "game/state.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/text.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "widget/city_draw_highway.h"

static int show_building_religion(const building *b)
{
    return
        b->type == BUILDING_ORACLE || b->type == BUILDING_LARARIUM || b->type == BUILDING_SMALL_TEMPLE_CERES ||
        b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
        b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS ||
        b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
        b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
        b->type == BUILDING_SMALL_MAUSOLEUM || b->type == BUILDING_LARGE_MAUSOLEUM ||
        b->type == BUILDING_LARGE_TEMPLE_VENUS || b->type == BUILDING_GRAND_TEMPLE_CERES ||
        b->type == BUILDING_GRAND_TEMPLE_NEPTUNE || b->type == BUILDING_GRAND_TEMPLE_MERCURY ||
        b->type == BUILDING_GRAND_TEMPLE_MARS || b->type == BUILDING_GRAND_TEMPLE_VENUS ||
        b->type == BUILDING_PANTHEON || b->type == BUILDING_NYMPHAEUM ||
        b->type == BUILDING_SHRINE_CERES || b->type == BUILDING_SHRINE_MARS ||
        b->type == BUILDING_SHRINE_MERCURY || b->type == BUILDING_SHRINE_VENUS ||
        b->type == BUILDING_SHRINE_NEPTUNE;
}

static int show_building_food_stocks(const building *b)
{
    return b->type == BUILDING_MARKET || b->type == BUILDING_WHARF || b->type == BUILDING_GRANARY ||
           b->type == BUILDING_CARAVANSERAI || b->type == BUILDING_MESS_HALL;
}

static int show_building_tax_income(const building *b)
{
    return b->type == BUILDING_FORUM || b->type == BUILDING_SENATE;
}

static int show_building_water(const building *b)
{
    return b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN || b->type == BUILDING_RESERVOIR ||
        (b->type == BUILDING_GRAND_TEMPLE_NEPTUNE && building_monument_gt_module_is_active(NEPTUNE_MODULE_2_CAPACITY_AND_WATER));
}

static int show_building_sentiment(const building *b)
{
    return b->house_size > 0;
}

static int show_building_roads(const building *b)
{
    return building_type_is_roadblock(b->type);
}

static int draw_top_roads(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 0;
    }
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return 0;
    }
    building *b = building_get(map_building_at(grid_offset));
    if (b->type != BUILDING_TRIUMPHAL_ARCH) {
        return 0;
    }
    int image_id = map_image_at(grid_offset);
    image_draw_isometric_top_from_draw_tile(image_id, x, y, COLOR_MASK_NONE, scale);
    const image *img = image_get(image_id);
    int animation_offset = building_animation_offset(b, image_id, grid_offset);
    if (animation_offset > 0) {
        int y_offset = img->top ? img->top->original.height - FOOTPRINT_HALF_HEIGHT : 0;
        if (animation_offset > img->animation->num_sprites) {
            animation_offset = img->animation->num_sprites;
        }
        image_draw(image_id + img->animation->start_offset + animation_offset,
            x + img->animation->sprite_offset_x,
            y + img->animation->sprite_offset_y - y_offset,
            COLOR_MASK_NONE, scale);
    }
    return 1;
}

static int show_building_mothball(const building *b)
{
    return b->state == BUILDING_STATE_MOTHBALLED;
}

static int show_building_logistics(const building *b)
{
    return b->type == BUILDING_WAREHOUSE || b->type == BUILDING_WAREHOUSE_SPACE ||
           b->type == BUILDING_GRANARY || b->type == BUILDING_DEPOT;
}

static int show_building_storages(const building *b)
{
    b = building_main((building *) b);
    return b->storage_id > 0 && building_storage_get(b->storage_id);
}

static int show_building_none(const building *b)
{
    return 0;
}


static int show_figure_religion(const figure *f)
{
    return f->type == FIGURE_PRIEST || f->type == FIGURE_PRIEST_SUPPLIER;
}

static int show_figure_efficiency(const figure *f)
{
    return f->type == FIGURE_CART_PUSHER || f->type == FIGURE_LABOR_SEEKER;
}

static int show_figure_food_stocks(const figure *f)
{
    if (f->type == FIGURE_MARKET_SUPPLIER || f->type == FIGURE_MARKET_TRADER ||
        f->type == FIGURE_CARAVANSERAI_SUPPLIER || f->type == FIGURE_CARAVANSERAI_COLLECTOR ||
        f->type == FIGURE_MESS_HALL_SUPPLIER || f->type == FIGURE_MESS_HALL_FORT_SUPPLIER || f->type == FIGURE_MESS_HALL_COLLECTOR ||
        f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
        return 1;
    } else if (f->type == FIGURE_CART_PUSHER) {
        return resource_is_food(f->resource_id);
    } else if (f->type == FIGURE_WAREHOUSEMAN) {
        building *b = building_get(f->building_id);
        return b->type == BUILDING_GRANARY;
    }
    return 0;
}

static int show_figure_tax_income(const figure *f)
{
    return f->type == FIGURE_TAX_COLLECTOR;
}

static int show_figure_logistics(const figure *f)
{
    return f->type == FIGURE_WAREHOUSEMAN || f->type == FIGURE_DEPOT_CART_PUSHER;
}

static int show_figure_employment(const figure *f)
{
    return f->type == FIGURE_LABOR_SEEKER;
}

static int show_figure_none(const figure *f)
{
    return 0;
}

static int get_column_height_religion(const building *b)
{
    return b->house_size && b->data.house.num_gods ? b->data.house.num_gods * 18 / 10 : NO_COLUMN;
}

static int get_column_height_efficiency(const building *b)
{
    int percentage = building_get_efficiency(b);
    if (percentage == -1) {
        return NO_COLUMN;
    }
    return calc_bound(percentage / 10, 1, 10);
}

static int get_column_height_food_stocks(const building *b)
{
    if (b->house_size && model_get_house(b->subtype.house_level)->food_types) {
        int pop = b->house_population;
        int stocks = 0;
        for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            if (resource_is_inventory(r)) {
                stocks += b->resources[r];
            }
        }
        int pct_stocks = calc_percentage(stocks, pop);
        if (pct_stocks <= 0) {
            return 10;
        } else if (pct_stocks < 100) {
            return 5;
        } else if (pct_stocks <= 200) {
            return 1;
        }
    }
    return NO_COLUMN;
}

static int get_column_height_levy(const building *b)
{
    int levy = building_get_levy(b);
    int height = calc_percentage(levy, PANTHEON_LEVY_MONTHLY) / 10;
    height = calc_bound(height, 1, 10);
    return levy ? height : NO_COLUMN;
}

static int get_column_height_tax_income(const building *b)
{
    if (b->house_size) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
        if (pct > 0) {
            return pct / 25;
        }
    }
    return NO_COLUMN;
}

static int get_column_height_employment(const building *b)
{
    int full_staff = building_get_laborers(b->type);
    int pct_staff = calc_percentage(b->num_workers, full_staff);

    int height = 100 - pct_staff;
    if (height == 0) {
        return NO_COLUMN;
    }
    return full_staff ? height / 10 : NO_COLUMN;
}

static int get_column_height_none(const building *b)
{
    return NO_COLUMN;
}

static void add_god(tooltip_context *c, int god_id)
{
    int index = c->num_extra_texts;
    c->extra_text_groups[index] = 59;
    c->extra_text_ids[index] = 11 + god_id;
    c->num_extra_texts++;
}

static int get_tooltip_religion(tooltip_context *c, const building *b)
{
    if (b->house_pantheon_access) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PANTHEON_ACCESS;
        return 0;
    }

    if (b->data.house.num_gods < 5) {
        if (b->data.house.temple_ceres) {
            add_god(c, GOD_CERES);
        }
        if (b->data.house.temple_neptune) {
            add_god(c, GOD_NEPTUNE);
        }
        if (b->data.house.temple_mercury) {
            add_god(c, GOD_MERCURY);
        }
        if (b->data.house.temple_mars) {
            add_god(c, GOD_MARS);
        }
        if (b->data.house.temple_venus) {
            add_god(c, GOD_VENUS);
        }
    }
    if (b->data.house.num_gods <= 0) {
        return 12;
    } else if (b->data.house.num_gods == 1) {
        return 13;
    } else if (b->data.house.num_gods == 2) {
        return 14;
    } else if (b->data.house.num_gods == 3) {
        return 15;
    } else if (b->data.house.num_gods == 4) {
        return 16;
    } else if (b->data.house.num_gods == 5) {
        return 17;
    } else {
        return 18; // >5 gods, shouldn't happen...
    }
}

static int get_tooltip_efficiency(tooltip_context *c, const building *b)
{
    int efficiency = building_get_efficiency(b);
    if (efficiency == -1) {
        return 0;
    }
    if (efficiency == 0) {
        c->translation_key = TR_TOOLTIP_OVERLAY_EFFICIENCY_0;
    }
    else if (efficiency < 25) {
        c->translation_key = TR_TOOLTIP_OVERLAY_EFFICIENCY_1;
    }
    else if (efficiency < 50) {
        c->translation_key = TR_TOOLTIP_OVERLAY_EFFICIENCY_2;
    }
    else if (efficiency < 80) {
        c->translation_key = TR_TOOLTIP_OVERLAY_EFFICIENCY_3;
    }
    else if (efficiency < 95) {
        c->translation_key = TR_TOOLTIP_OVERLAY_EFFICIENCY_4;
    }
    else {
        c->translation_key = TR_TOOLTIP_OVERLAY_EFFICIENCY_5;
    }
    return 0;
}

static int get_tooltip_food_stocks(tooltip_context *c, const building *b)
{
    if (b->house_population <= 0) {
        return 0;
    }
    if (!model_get_house(b->subtype.house_level)->food_types) {
        return 104;
    } else {
        int stocks_present = 0;
        for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            if (resource_is_inventory(r)) {
                stocks_present += b->resources[r];
            }
        }
        int stocks_per_pop = calc_percentage(stocks_present, b->house_population);
        if (stocks_per_pop <= 0) {
            return 4;
        } else if (stocks_per_pop < 100) {
            return 5;
        } else if (stocks_per_pop <= 200) {
            return 6;
        } else {
            return 7;
        }
    }
}

static int get_tooltip_tax_income(tooltip_context *c, const building *b)
{
    int denarii = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
    if (denarii > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = denarii;
        return 45;
    } else if (b->house_tax_coverage > 0) {
        return 44;
    } else {
        return 43;
    }
}

static int get_tooltip_employment(tooltip_context *c, const building *b)
{
    int full = building_get_laborers(b->type);
    int missing = full - b->num_workers;
    
    if (full >= 1) {
        if (missing == 0) {
            c->translation_key = TR_TOOLTIP_OVERLAY_EMPLOYMENT_FULL;
        } else if (missing <= 1) {
            c->has_numeric_prefix = 1;
            c->numeric_prefix = missing;
            c->translation_key = TR_TOOLTIP_OVERLAY_EMPLOYMENT_MISSING_1;
            return 1;
        } else if (missing >= 2 && b->state == BUILDING_STATE_MOTHBALLED) {
            c->has_numeric_prefix = 1;
            c->numeric_prefix = missing;
            c->translation_key = TR_TOOLTIP_OVERLAY_EMPLOYMENT_MOTHBALL;
            return 1;
        } else {
            c->has_numeric_prefix = 1;
            c->numeric_prefix = missing;
            c->translation_key = TR_TOOLTIP_OVERLAY_EMPLOYMENT_MISSING_2;
            return 1;
        }
    }
    return 0;
}

static int get_tooltip_water(tooltip_context *c, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE)) {
        if (map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
            return 2;
        } else {
            return 1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
        return 3;
    }
    return 0;
}

static int get_tooltip_desirability(tooltip_context *c, int grid_offset)
{
    int desirability;
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        desirability = b->desirability;
    } else {
        desirability = map_desirability_get(grid_offset);
    }
    if (desirability < 0) {
        return 91;
    } else if (desirability == 0) {
        return 92;
    } else {
        return 93;
    }
}

static int get_tooltip_none(tooltip_context *c, int grid_offset)
{
    return 0;
}

static int get_tooltip_levy(tooltip_context *c, const building *b)
{
    int levy = building_get_levy(b);
    if (levy > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = levy;
        c->translation_key = TR_TOOLTIP_OVERLAY_LEVY;
        return 1;
    }
    return 0;
}

static int get_tooltip_sentiment(tooltip_context *c, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return 0;
    }
    int building_id = map_building_at(grid_offset);
    building *b = building_get(building_id);
    if (!b || !b->house_population) {
        return 0;
    }
    int happiness = b->sentiment.house_happiness;
    int sentiment_text_id = TR_BUILDING_WINDOW_HOUSE_SENTIMENT_1;
    if (happiness > 0) {
        sentiment_text_id = happiness / 10 + TR_BUILDING_WINDOW_HOUSE_SENTIMENT_2;
    }
    c->translation_key = sentiment_text_id;
    return 1;
}


const city_overlay *city_overlay_for_religion(void)
{
    static city_overlay overlay = {
        OVERLAY_RELIGION,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_religion,
        show_figure_religion,
        get_column_height_religion,
        0,
        get_tooltip_religion,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_efficiency(void)
{
    static city_overlay overlay = {
        OVERLAY_EFFICIENCY,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_roads,
        show_figure_efficiency,
        get_column_height_efficiency,
        0,
        get_tooltip_efficiency,
        0,
        draw_top_roads
    };
    return &overlay;
}

const city_overlay *city_overlay_for_food_stocks(void)
{
    static city_overlay overlay = {
        OVERLAY_FOOD_STOCKS,
        COLUMN_COLOR_RED,
        show_building_food_stocks,
        show_figure_food_stocks,
        get_column_height_food_stocks,
        0,
        get_tooltip_food_stocks,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_tax_income(void)
{
    static city_overlay overlay = {
        OVERLAY_TAX_INCOME,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_tax_income,
        show_figure_tax_income,
        get_column_height_tax_income,
        0,
        get_tooltip_tax_income,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_employment(void)
{
    static city_overlay overlay = {
        OVERLAY_EMPLOYMENT,
        COLUMN_COLOR_RED_TO_GREEN,
        show_building_none,
        show_figure_employment,
        get_column_height_employment,
        0,
        get_tooltip_employment,
        0,
        0
    };
    return &overlay;
}

static int has_deleted_building(int grid_offset)
{
    building *b = building_get(map_building_at(grid_offset));
    b = building_main(b);
    return b->id && (b->is_deleted || map_property_is_deleted(b->grid_offset));
}

static int terrain_on_water_overlay(void)
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_MEADOW |
        TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_HIGHWAY;
}

static int draw_footprint_water(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 1;
    }
    int is_building = map_terrain_is(grid_offset, TERRAIN_BUILDING);
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY) && !map_terrain_is(grid_offset, TERRAIN_GATEHOUSE)) {
        city_draw_highway_footprint(x, y, scale, grid_offset);
    } else if (map_terrain_is(grid_offset, terrain_on_water_overlay()) && !is_building) {
        image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0, scale);
    } else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        // display grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0, scale);
    } else if (is_building) {
        building *b = building_get(map_building_at(grid_offset));
        int terrain = map_terrain_get(grid_offset);
        if (b->id && (b->has_well_access || (b->house_size && b->has_water_access))) {
            terrain |= TERRAIN_FOUNTAIN_RANGE;
        }
        int image_offset;
        switch (terrain & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
            case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
                image_offset = 24;
                break;
            case TERRAIN_RESERVOIR_RANGE:
                image_offset = 8;
                break;
            case TERRAIN_FOUNTAIN_RANGE:
                image_offset = 16;
                break;
            default:
                image_offset = 0;
                break;
        }
        city_with_overlay_draw_building_footprint(x, y, grid_offset, image_offset);
    } else {
        int image_id = image_group(GROUP_TERRAIN_OVERLAY);
        switch (map_terrain_get(grid_offset) & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
            case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
                image_id += 27;
                break;
            case TERRAIN_RESERVOIR_RANGE:
                image_id += 11;
                break;
            case TERRAIN_FOUNTAIN_RANGE:
                image_id += 19;
                break;
            default:
                image_id = map_image_at(grid_offset);
                break;
        }
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0, scale);
    }
    return 1;
}

static int draw_top_water(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 1;
    }
    if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            color_t color_mask = 0;
            if (map_property_is_deleted(grid_offset) && map_property_multi_tile_size(grid_offset) == 1) {
                color_mask = COLOR_MASK_RED;
            }
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_building_at(grid_offset)) {
        city_with_overlay_draw_building_top(x, y, grid_offset);
    }
    return 1;
}

const city_overlay *city_overlay_for_water(void)
{
    static city_overlay overlay = {
        OVERLAY_WATER,
        COLUMN_COLOR_GREEN,
        show_building_water,
        show_figure_none,
        get_column_height_none,
        get_tooltip_water,
        0,
        draw_footprint_water,
        draw_top_water
    };
    return &overlay;
}

static color_t get_sentiment_color(int sentiment)
{
    sentiment = calc_bound(sentiment - 50, -40, 40);
    color_t color = COLOR_OVERLAY_NEUTRAL;
    if (sentiment < 0) {
        color += COLOR_OVERLAY_NEGATIVE_STEP * (sentiment - 10);
    } else {
        color -= COLOR_OVERLAY_POSITIVE_STEP * (sentiment + 10);
    }
    return color;
}

static void blend_color_to_footprint(int x, int y, int size, color_t color, float scale)
{
    int total_steps = size * 2 - 1;
    int tiles = 1;

    for (int step = 1; step <= total_steps; step++) {
        if (tiles % 2) {
            image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, color, scale);
        }
        int y_offset = 15 + 15 * (tiles % 2);

        for (int i = 1; i <= tiles / 2; i++) {
            image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y - y_offset, color, scale);
            image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y + y_offset, color, scale);
            y_offset += 30;
        }
        x += 30;
        step < size ? tiles++ : tiles--;
    }
}

static int draw_sentiment_footprint(int x, int y, float scale, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return 0;
    }
    int building_id = map_building_at(grid_offset);
    building *b = building_get(building_id);
    if (!b || !b->house_population || b->is_deleted || map_property_is_deleted(b->grid_offset)) {
        return 0;
    }
    if (map_property_is_draw_tile(grid_offset)) {
        city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
        blend_color_to_footprint(x, y, b->house_size, get_sentiment_color(b->sentiment.house_happiness), scale);
    }
    return 1;
}

static int draw_sentiment_top(int x, int y, float scale, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return 0;
    }
    int building_id = map_building_at(grid_offset);
    building *b = building_get(building_id);
    if (!b || !b->house_population || b->is_deleted || map_property_is_deleted(b->grid_offset)) {
        return 0;
    }
    if (map_property_is_draw_tile(grid_offset)) {
        city_with_overlay_draw_building_top(x, y, grid_offset);
        color_t color = get_sentiment_color(b->sentiment.house_happiness);
        image_draw_set_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color, scale);
    }
    return 1;
}

const city_overlay *city_overlay_for_sentiment(void)
{
    static city_overlay overlay = {
        OVERLAY_SENTIMENT,
        COLUMN_COLOR_GREEN,
        show_building_sentiment,
        show_figure_none,
        get_column_height_none,
        get_tooltip_sentiment,
        0,
        draw_sentiment_footprint,
        draw_sentiment_top,
        0
    };
    return &overlay;
}

static int terrain_on_desirability_overlay(void)
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ROAD |
        TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE |
        TERRAIN_HIGHWAY;
}

static int get_desirability_image_offset(int desirability)
{
    if (desirability < -10) {
        return 0;
    } else if (desirability < -5) {
        return 1;
    } else if (desirability < 0) {
        return 2;
    } else if (desirability == 1) {
        return 3;
    } else if (desirability < 5) {
        return 4;
    } else if (desirability < 10) {
        return 5;
    } else if (desirability < 15) {
        return 6;
    } else if (desirability < 20) {
        return 7;
    } else if (desirability < 25) {
        return 8;
    } else {
        return 9;
    }
}

static int draw_footprint_desirability(int x, int y, float scale, int grid_offset)
{
    color_t color_mask = map_property_is_deleted(grid_offset) ? COLOR_MASK_RED : 0;
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY) && !map_terrain_is(grid_offset, TERRAIN_GATEHOUSE)) {
        city_draw_highway_footprint(x, y, scale, grid_offset);
    } else if (map_terrain_is(grid_offset, terrain_on_desirability_overlay())
        && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display empty land/grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, color_mask, scale);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        if (has_deleted_building(grid_offset)) {
            color_mask = COLOR_MASK_RED;
        }
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        int offset = get_desirability_image_offset(b->desirability);
        image_draw_isometric_footprint_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
            color_mask, scale);
    } else if (map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        image_draw_isometric_footprint_from_draw_tile(
            image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y, color_mask, scale);
    } else {
        image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
    }
    return 1;
}

static int draw_top_desirability(int x, int y, float scale, int grid_offset)
{
    color_t color_mask = map_property_is_deleted(grid_offset) ? COLOR_MASK_RED : 0;
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay())
        && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // grass, no top needed
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        if (has_deleted_building(grid_offset)) {
            color_mask = COLOR_MASK_RED;
        }
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        int offset = get_desirability_image_offset(b->desirability);
        image_draw_isometric_top_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
            color_mask, scale);
    } else if (map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        image_draw_isometric_top_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
            color_mask, scale);
    } else {
        image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
    }
    return 1;
}

const city_overlay *city_overlay_for_desirability(void)
{
    static city_overlay overlay = {
        OVERLAY_DESIRABILITY,
        COLUMN_COLOR_GREEN,
        show_building_none,
        show_figure_none,
        get_column_height_none,
        get_tooltip_desirability,
        0,
        draw_footprint_desirability,
        draw_top_desirability
    };
    return &overlay;
}

const city_overlay *city_overlay_for_roads(void)
{
    static city_overlay overlay = {
        OVERLAY_ROADS,
        COLUMN_COLOR_GREEN,
        show_building_roads,
        show_figure_none,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        draw_top_roads
    };
    return &overlay;
}

const city_overlay *city_overlay_for_levy(void)
{
    static city_overlay overlay = {
        OVERLAY_LEVY,
        COLUMN_COLOR_GREEN,
        show_building_none,
        show_figure_none,
        get_column_height_levy,
        0,
        get_tooltip_levy,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_mothball(void)
{
    static city_overlay overlay = {
        OVERLAY_MOTHBALL,
        COLUMN_COLOR_GREEN,
        show_building_mothball,
        show_figure_none,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_logistics(void)
{
    static city_overlay overlay = {
        OVERLAY_LOGISTICS,
        COLUMN_COLOR_GREEN,
        show_building_logistics,
        show_figure_logistics,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        0
    };
    return &overlay;
}

static void draw_storage_ids(int x, int y, float scale, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return;
    }
    int building_id = map_building_at(grid_offset);
    building *b = building_get(building_id);
    if (!b || b->is_deleted || map_property_is_deleted(b->grid_offset) || !b->storage_id ||
        !map_property_is_draw_tile(grid_offset)) {
        return;
    }
    uint8_t number[10];
    string_from_int(number, b->storage_id, 0);
    int text_width = text_get_width(number, FONT_SMALL_PLAIN);
    int box_width = text_width + 10;
    int box_height = 22;
    if (b->type == BUILDING_GRANARY) {
        x += 90;
        y += 15;
    } else if (b->type == BUILDING_WAREHOUSE) {
        switch (building_rotation_get_building_orientation(b->subtype.orientation)) {
            case 6:
                x -= 30;
                break;
            case 4:
                x += 30;
                y -= 30;
                break;
            case 2:
                x += 90;
                break;
            case 0:
            default:
                x += 30;
                y += 30;
                break;
        }
    }
    x = (int) (x / scale);
    y = (int) (y / scale);
    x -= box_width / 2;
    y -= box_height / 2;
    graphics_draw_rect(x, y, box_width, box_height, COLOR_BLACK);
    graphics_fill_rect(x + 1, y + 1, box_width - 2, box_height - 2, COLOR_WHITE);
    text_draw(number, x + 5, y + 6, FONT_SMALL_PLAIN, COLOR_BLACK);
}

const city_overlay *city_overlay_for_storages(void)
{
    static city_overlay overlay = {
        OVERLAY_STORAGES,
        COLUMN_COLOR_GREEN,
        show_building_storages,
        show_figure_none,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        0,
        draw_storage_ids
    };
    return &overlay;
}
