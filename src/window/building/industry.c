#include "industry.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/count.h"
#include "building/industry.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/file.h"
#include "core/string.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/mouse.h"
#include "scenario/allowed_building.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/popup_dialog.h"

#include <stdio.h>

static void set_city_mint_conversion(const generic_button *button);

static generic_button mint_conversion_buttons[] = {
    {0, 0, 432, 24, set_city_mint_conversion, 0, RESOURCE_DENARII},
    {0, 24, 432, 24, set_city_mint_conversion, 0, RESOURCE_GOLD},
};

static struct {
    int city_mint_id;
    unsigned int focus_button_id;
} data;

static void draw_farm(building_info_context *c, int help_id, const char *sound_file, int group_id, int resource)
{
    c->advisor_button = ADVISOR_TRADE;
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    image_draw(resource_get_data(resource)->image.icon, c->x_offset + 10, c->y_offset + 10,
        COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    int pct_grown = calc_percentage(b->data.industry.progress, building_industry_get_max_progress(b));
    int width = lang_text_draw(group_id, 2, c->x_offset + 32, c->y_offset + 44, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_grown, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);

    int efficiency = building_get_efficiency(b);
    if (efficiency < 0) {
        efficiency = 0;
    }

    width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_WINDOW_INDUSTRY_EFFICIENCY,
        c->x_offset + 32, c->y_offset + 70, FONT_NORMAL_BLACK);
    text_draw_percentage(efficiency, c->x_offset + 32 + width, c->y_offset + 70,
        efficiency >= 50 ? FONT_NORMAL_BLACK : FONT_NORMAL_RED);

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 96, 69, 25);
    } else if (city_resource_is_mothballed(resource)) {
        window_building_draw_description_at(c, 96, group_id, 4);
    } else if (b->data.industry.curse_days_left > 4) {
        window_building_draw_description_at(c, 96, group_id, 11);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 96, group_id, 5);
    } else if (c->worker_percentage < 25) {
        window_building_draw_description_at(c, 96, group_id, 10);
    } else if (c->worker_percentage < 50) {
        window_building_draw_description_at(c, 96, group_id, 9);
    } else if (c->worker_percentage < 75) {
        window_building_draw_description_at(c, 96, group_id, 8);
    } else if (c->worker_percentage < 100) {
        window_building_draw_description_at(c, 96, group_id, 7);
    } else if (efficiency < 80) {
        window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION,
            TR_BUILDING_WINDOW_INDUSTRY_LOW_EFFICIENCY_RAW_MATERIALS);
    } else if (c->worker_percentage >= 100) {
        window_building_draw_description_at(c, 96, group_id, 6);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 162, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 168);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 170);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 110, group_id, 1);
}

void window_building_draw_wheat_farm(building_info_context *c)
{
    draw_farm(c, 89, "wavs/wheat_farm.wav", 112, RESOURCE_WHEAT);
}

void window_building_draw_vegetable_farm(building_info_context *c)
{
    draw_farm(c, 90, "wavs/veg_farm.wav", 113, RESOURCE_VEGETABLES);
}

void window_building_draw_fruit_farm(building_info_context *c)
{
    draw_farm(c, 90, "wavs/figs_farm.wav", 114, RESOURCE_FRUIT);
}

void window_building_draw_olive_farm(building_info_context *c)
{
    draw_farm(c, 91, "wavs/olives_farm.wav", 115, RESOURCE_OLIVES);
}

void window_building_draw_vines_farm(building_info_context *c)
{
    draw_farm(c, 91, "wavs/vines_farm.wav", 116, RESOURCE_VINES);
}

void window_building_draw_pig_farm(building_info_context *c)
{
    draw_farm(c, 90, "wavs/meat_farm.wav", 117, RESOURCE_MEAT);
}

static void draw_raw_material(
    building_info_context *c, int help_id, const char *sound_file, int group_id, int text_offset, int resource)
{
    c->advisor_button = ADVISOR_TRADE;
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    image_draw(resource_get_data(resource)->image.icon, c->x_offset + 10, c->y_offset + 10,
        COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw_centered(group_id, text_offset, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    int pct_done = calc_percentage(b->data.industry.progress, building_industry_get_max_progress(b));
    int width = lang_text_draw(group_id, text_offset + 2, c->x_offset + 32, c->y_offset + 44, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, text_offset + 3, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);

    int efficiency = building_get_efficiency(b);
    if (efficiency < 0) {
        efficiency = 0;
    }

    width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_WINDOW_INDUSTRY_EFFICIENCY,
        c->x_offset + 32, c->y_offset + 70, FONT_NORMAL_BLACK);
    text_draw_percentage(efficiency, c->x_offset + 32 + width, c->y_offset + 70,
        efficiency >= 50 ? FONT_NORMAL_BLACK : FONT_NORMAL_RED);

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 96, 69, 25);
    } else if (b->strike_duration_days > 0) {
        window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_WINDOW_BUILDING_WORKSHOP_STRIKING);
    } else if (city_resource_is_mothballed(resource)) {
        window_building_draw_description_at(c, 96, group_id, text_offset + 4);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 96, group_id, text_offset + 5);
    } else if (c->worker_percentage < 25) {
        window_building_draw_description_at(c, 96, group_id, text_offset + 10);
    } else if (c->worker_percentage < 50) {
        window_building_draw_description_at(c, 96, group_id, text_offset + 9);
    } else if (c->worker_percentage < 75) {
        window_building_draw_description_at(c, 96, group_id, text_offset + 8);
    } else if (c->worker_percentage < 100) {
        window_building_draw_description_at(c, 96, group_id, text_offset + 7);
    } else if (efficiency < 80) {
        window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION,
            TR_BUILDING_WINDOW_INDUSTRY_LOW_EFFICIENCY_RAW_MATERIALS);
    } else {
        window_building_draw_description_at(c, 96, group_id, text_offset + 6);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 162, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 168);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 170);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 110, group_id, text_offset + 1);
}

void window_building_draw_marble_quarry(building_info_context *c)
{
    draw_raw_material(c, 95, "wavs/quarry.wav", 118, 0, RESOURCE_MARBLE);
}

void window_building_draw_iron_mine(building_info_context *c)
{
    draw_raw_material(c, 93, "wavs/mine.wav", 119, 0, RESOURCE_IRON);
}

void window_building_draw_gold_mine(building_info_context *c)
{
    draw_raw_material(c, 93, "wavs/mine.wav", CUSTOM_TRANSLATION, TR_BUILDING_GOLD_MINE, RESOURCE_GOLD);
}

void window_building_draw_stone_quarry(building_info_context *c)
{
    draw_raw_material(c, 93, "wavs/quarry.wav", CUSTOM_TRANSLATION, TR_BUILDING_STONE_QUARRY, RESOURCE_STONE);
}

void window_building_draw_sand_pit(building_info_context *c)
{
    draw_raw_material(c, 93, "wavs/clay.wav", CUSTOM_TRANSLATION, TR_BUILDING_SAND_PIT, RESOURCE_SAND);
}

void window_building_draw_timber_yard(building_info_context *c)
{
    draw_raw_material(c, 94, "wavs/timber.wav", 120, 0, RESOURCE_TIMBER);
}

void window_building_draw_clay_pit(building_info_context *c)
{
    draw_raw_material(c, 92, "wavs/clay.wav", 121, 0, RESOURCE_CLAY);
}

static int no_target_for_resource(const building *b, resource_type resource)
{
    return !resource_is_storable(resource) && b->data.industry.progress == 0 &&
        !building_has_workshop_for_raw_material_with_room(resource, b->road_network_id) &&
        !building_monument_get_monument(b->x, b->y, resource, b->road_network_id, 0);
}

static void draw_workshop(
    building_info_context *c, int help_id, const char *sound_file, int group_id, int text_offset, resource_type resource)
{
    c->advisor_button = ADVISOR_TRADE;
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    image_draw(resource_get_data(resource)->image.icon, c->x_offset + 10, c->y_offset + 10,
        COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw_centered(group_id, text_offset, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    int pct_done = calc_percentage(b->data.industry.progress, building_industry_get_max_progress(b));
    int width = lang_text_draw(group_id, text_offset + 2, c->x_offset + 32, c->y_offset + 40, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 40, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, text_offset + 3, c->x_offset + 32 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

    int resources_y_offset = 0;

    if (!b->strike_duration_days) {
        resource_supply_chain chain[RESOURCE_SUPPLY_CHAIN_MAX_SIZE];
        int num_raw_materials = resource_get_supply_chain_for_good(chain, resource);
        if (num_raw_materials > 0) {
            width = 0;
            for (int i = 0; i < num_raw_materials; i++) {
                int current_width = lang_text_get_width(group_id, text_offset + 12 + i, FONT_NORMAL_BLACK);
                if (current_width > width) {
                    width = current_width;
                }
            }
            for (int i = 0; i < num_raw_materials; i++) {
                font_t font = chain[i].raw_amount > b->resources[chain[i].raw_material] ?
                    FONT_NORMAL_RED : FONT_NORMAL_BLACK;
                image_draw(resource_get_data(chain[i].raw_material)->image.icon,
                    c->x_offset + 32, c->y_offset + 56 + resources_y_offset, COLOR_MASK_NONE, SCALE_NONE);
                lang_text_draw(group_id, text_offset + 12 + i,
                    c->x_offset + 60, c->y_offset + 60 + resources_y_offset, FONT_NORMAL_BLACK);
                int extra_width = lang_text_draw_amount(8, 10, b->resources[chain[i].raw_material],
                    c->x_offset + 60 + width, c->y_offset + 60 + resources_y_offset, font);
                text_draw_number_scaled(chain[i].raw_amount, '(',
                    translation_for(TR_BUILDING_WINDOW_INDUSTRY_NEEDED),
                    c->x_offset + 60 + width + extra_width, c->y_offset + 60 + resources_y_offset,
                    FONT_NORMAL_BLACK, COLOR_MASK_NONE, SCALE_NONE);
                resources_y_offset += 20;
            }
        }
    }

    int efficiency = building_get_efficiency(b);
    if (efficiency < 0) {
        efficiency = 0;
    }

    width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_WINDOW_INDUSTRY_EFFICIENCY,
        c->x_offset + 32, c->y_offset + 78 + resources_y_offset, FONT_NORMAL_BLACK);
    text_draw_percentage(efficiency, c->x_offset + 32 + width, c->y_offset + 78 + resources_y_offset,
        efficiency >= 50 ? FONT_NORMAL_BLACK : FONT_NORMAL_RED);

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 96 + resources_y_offset, 69, 25);
    } 
    else if (b->strike_duration_days > 0) {
        window_building_draw_description_at(c, 96 + resources_y_offset, CUSTOM_TRANSLATION,
            TR_WINDOW_BUILDING_WORKSHOP_STRIKING);
    } else if (city_resource_is_mothballed(resource)) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 4);
    } else if (no_target_for_resource(b, resource)) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 13);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 5);
    } else if (!building_industry_has_raw_materials_for_production(b) ||
        (b->type == BUILDING_CONCRETE_MAKER && !b->has_water_access)) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 11);
    } else if (c->worker_percentage < 25) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 10);
    } else if (c->worker_percentage < 50) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 9);
    } else if (c->worker_percentage < 75) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 8);
    } else if (c->worker_percentage < 100) {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 7);
    } else if (b->type == BUILDING_CONCRETE_MAKER && b->has_water_access == 1) {
        window_building_draw_description_at(c, 96 + resources_y_offset, CUSTOM_TRANSLATION,
            TR_BUILDING_CONCRETE_MAKER_IMPROVE_WATER_ACCESS);
    } else if (efficiency < 70) {
        window_building_draw_description_at(c, 96 + resources_y_offset, CUSTOM_TRANSLATION,
            TR_BUILDING_WINDOW_INDUSTRY_LOW_EFFICIENCY_WORKSHOPS);
    } else {
        window_building_draw_description_at(c, 96 + resources_y_offset, group_id, text_offset + 6);
    }

    if (b->type == BUILDING_CONCRETE_MAKER) {
        // Concrete maker needs more text space
        resources_y_offset += 2 * BLOCK_SIZE;
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 146 + resources_y_offset, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 152 + resources_y_offset);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76,
        c->y_offset + 154 + resources_y_offset);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 126 + resources_y_offset,
        group_id, text_offset + 1);
}

void window_building_draw_wine_workshop(building_info_context *c)
{
    draw_workshop(c, 96, "wavs/wine_workshop.wav", 122, 0, RESOURCE_WINE);
}

void window_building_draw_oil_workshop(building_info_context *c)
{
    draw_workshop(c, 97, "wavs/oil_workshop.wav", 123, 0, RESOURCE_OIL);
}

void window_building_draw_weapons_workshop(building_info_context *c)
{
    draw_workshop(c, 98, "wavs/weapons_workshop.wav", 124, 0, RESOURCE_WEAPONS);
}

void window_building_draw_furniture_workshop(building_info_context *c)
{
    draw_workshop(c, 99, "wavs/furniture_workshop.wav", 125, 0, RESOURCE_FURNITURE);
}

void window_building_draw_pottery_workshop(building_info_context *c)
{
    draw_workshop(c, 1, "wavs/pottery_workshop.wav", 126, 0, RESOURCE_POTTERY);
}

void window_building_draw_brickworks(building_info_context *c)
{
    draw_workshop(c, 1, ASSETS_DIRECTORY "/Sounds/Brickworks.ogg", CUSTOM_TRANSLATION,
        TR_BUILDING_BRICKWORKS, RESOURCE_BRICKS);
}

void window_building_draw_concrete_maker(building_info_context *c)
{
    draw_workshop(c, 1, ASSETS_DIRECTORY "/Sounds/ConcreteMaker.ogg", CUSTOM_TRANSLATION, TR_BUILDING_CONCRETE_MAKER, RESOURCE_CONCRETE);
}

static int governor_palace_is_allowed(void)
{
    return scenario_allowed_building(BUILDING_GOVERNORS_HOUSE) || scenario_allowed_building(BUILDING_GOVERNORS_VILLA) ||
        scenario_allowed_building(BUILDING_GOVERNORS_PALACE);
}

void window_building_draw_city_mint(building_info_context *c)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/coin.wav");
    building *b = building_get(c->building_id);
    data.city_mint_id = 0;
    if (b->monument.phase == MONUMENT_FINISHED) {
        c->advisor_button = ADVISOR_FINANCIAL;
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
        image_draw(resource_get_data(RESOURCE_DENARII)->image.icon, c->x_offset + 10, c->y_offset + 10,
            COLOR_MASK_NONE, SCALE_NONE);

        int pct_done = calc_percentage(b->data.industry.progress, building_industry_get_max_progress(b));
        int width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_GOLD_MINE_PRODUCTION,
            c->x_offset + 32, c->y_offset + 40, FONT_NORMAL_BLACK);
        width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 40, FONT_NORMAL_BLACK);
        lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_GOLD_MINE_COMPLETE,
            c->x_offset + 32 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

        image_draw(resource_get_data(RESOURCE_GOLD)->image.icon, c->x_offset + 32, c->y_offset + 56,
            COLOR_MASK_NONE, SCALE_NONE);
        width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_STORED_GOLD,
            c->x_offset + 60, c->y_offset + 60, FONT_NORMAL_BLACK);
        lang_text_draw_amount(8, 10, b->resources[RESOURCE_GOLD],
            c->x_offset + 60 + width, c->y_offset + 60, FONT_NORMAL_BLACK);

        int efficiency = building_get_efficiency(b);
        if (efficiency < 0) {
            efficiency = 0;
        }

        width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_WINDOW_INDUSTRY_EFFICIENCY,
            c->x_offset + 32, c->y_offset + 78, FONT_NORMAL_BLACK);
        text_draw_percentage(efficiency, c->x_offset + 32 + width, c->y_offset + 78,
            efficiency >= 50 ? FONT_NORMAL_BLACK : FONT_NORMAL_RED);

        if (!c->has_road_access) {
            window_building_draw_description_at(c, 96, 69, 25);
        } else if (building_count_active(BUILDING_SENATE) == 0) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_NO_SENATE);
        } else if (b->num_workers <= 0) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_NO_EMPLOYEES);
        } else if (b->resources[RESOURCE_GOLD] < BUILDING_INDUSTRY_CITY_MINT_GOLD_PER_COIN &&
            b->output_resource_id == RESOURCE_DENARII) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_NO_GOLD);
        } else if (c->worker_percentage < 25) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_FEW_EMPLOYEES);
        } else if (c->worker_percentage < 50) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_SOME_EMPLOYEES);
        } else if (c->worker_percentage < 75) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_HALF_EMPLOYEES);
        } else if (c->worker_percentage < 100) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_MANY_EMPLOYEES);
        } else if (efficiency < 70) {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION,
                TR_BUILDING_WINDOW_INDUSTRY_LOW_EFFICIENCY_WORKSHOPS);
        } else {
            window_building_draw_description_at(c, 96, CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_FULL_EMPLOYEES);
        }

        inner_panel_draw(c->x_offset + 16, c->y_offset + 146, c->width_blocks - 2, 4);
        window_building_draw_employment(c, 152);
        window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 154);
        window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 190, CUSTOM_TRANSLATION,
            b->output_resource_id == RESOURCE_DENARII ?
                TR_BUILDING_CITY_MINT_DESC : TR_BUILDING_CITY_MINT_DESC_ALTERNATIVE);
        if (governor_palace_is_allowed() && b->output_resource_id == RESOURCE_DENARII) {
            window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 154,
                CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_DESC_NO_PALACE + city_buildings_has_governor_house());
        }
        lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_CONVERT,
            c->x_offset + 16, c->y_offset + BLOCK_SIZE * c->height_blocks - 114, FONT_NORMAL_BLACK);
        lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_GOLD_TO_DN,
            c->x_offset + 40, c->y_offset + BLOCK_SIZE * c->height_blocks - 90, FONT_NORMAL_BLACK);
        lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT_DN_TO_GOLD,
            c->x_offset + 40, c->y_offset + BLOCK_SIZE * c->height_blocks - 66, FONT_NORMAL_BLACK);
        data.city_mint_id = b->id;
    } else {
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
        window_building_draw_monument_construction_process(c, TR_BUILDING_CITY_MINT_PHASE_1,
            TR_BUILDING_CITY_MINT_PHASE_1_TEXT, TR_BUILDING_MONUMENT_CONSTRUCTION_DESC);
    }
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUILDING_CITY_MINT, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
}

void window_building_draw_city_mint_foreground(building_info_context *c)
{
    if (!data.city_mint_id) {
        return;
    }
    int x = c->x_offset + 16;
    int y = c->y_offset + BLOCK_SIZE * c->height_blocks - 94;
    button_border_draw(x, y, 20, 20, data.focus_button_id == 1);
    button_border_draw(x, y + 24, 20, 20, data.focus_button_id == 2);
    int selected_offset = building_get(data.city_mint_id)->output_resource_id == RESOURCE_DENARII ? 0 : 24;
    image_draw(assets_get_image_id("UI", "Denied_Walker_Checkmark"), x + 4, y + 4 + selected_offset, COLOR_MASK_NONE, SCALE_NONE);
}

static int shipyard_boats_needed(void)
{
    for (const building *wharf = building_first_of_type(BUILDING_WHARF); wharf; wharf = wharf->next_of_type) {
        if (wharf->num_workers > 0 && !wharf->data.industry.fishing_boat_id) {
            return 1;
        }
    }
    return 0;
}

void window_building_draw_shipyard(building_info_context *c)
{
    c->help_id = 82;
    window_building_play_sound(c, "wavs/shipyard.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(100, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else {
        int pct_done = calc_percentage(b->data.industry.progress, 160);
        int width = lang_text_draw(100, 2, c->x_offset + 32, c->y_offset + 56, FONT_NORMAL_BLACK);
        width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 56, FONT_NORMAL_BLACK);
        lang_text_draw(100, 3, c->x_offset + 32 + width, c->y_offset + 56, FONT_NORMAL_BLACK);
        if (shipyard_boats_needed()) {
            lang_text_draw_multiline(100, 5, c->x_offset + 32, c->y_offset + 80,
                BLOCK_SIZE * (c->width_blocks - 6), FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_multiline(100, 4, c->x_offset + 32, c->y_offset + 80,
                BLOCK_SIZE * (c->width_blocks - 6), FONT_NORMAL_BLACK);
        }
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 144);
}

void window_building_draw_wharf(building_info_context *c)
{
    c->help_id = 84;
	c->advisor_button = ADVISOR_TRADE;
    window_building_play_sound(c, "wavs/wharf.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(102, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    image_draw(resource_get_data(RESOURCE_FISH)->image.icon,
        c->x_offset + 10, c->y_offset + 10, COLOR_MASK_NONE, SCALE_NONE);

    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (city_resource_is_mothballed(RESOURCE_FISH)) {
        window_building_draw_description(c, CUSTOM_TRANSLATION, TR_WINDOW_BUILDING_WHARF_MOTHBALLED);
    } else if (!b->data.industry.fishing_boat_id) {
        window_building_draw_description(c, 102, 2);
    } else {
        int text_id;
        switch (figure_get(b->data.industry.fishing_boat_id)->action_state) {
            case FIGURE_ACTION_191_FISHING_BOAT_GOING_TO_FISH: text_id = 3; break;
            case FIGURE_ACTION_192_FISHING_BOAT_FISHING: text_id = 4; break;
            case FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF: text_id = 5; break;
            case FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF: text_id = 6; break;
            case FIGURE_ACTION_195_FISHING_BOAT_RETURNING_WITH_FISH: text_id = 7; break;
            default: text_id = 8; break;
        }
        window_building_draw_description(c, 102, text_id);
    }

    int width = lang_text_draw(CUSTOM_TRANSLATION, TR_BUILDING_WINDOW_INDUSTRY_WHARF_AVERAGE_CATCH,
        c->x_offset + 16, c->y_offset + 110, FONT_NORMAL_BLACK);
    width += text_draw_number(b->data.industry.average_production_per_month, '@', "",
        c->x_offset + 16 + width, c->y_offset + 110, FONT_NORMAL_BLACK, 0);
    image_draw(resource_get_data(RESOURCE_FISH)->image.icon, c->x_offset + 16 + width, c->y_offset + 110,
        COLOR_MASK_NONE, SCALE_NONE);

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 144);
}

static void city_mint_conversion_changed(int accepted, int checked)
{
    if (!accepted) {
        return;
    }
    building *city_mint = building_get(data.city_mint_id);
    if (city_mint->output_resource_id == RESOURCE_DENARII) {
        city_mint->output_resource_id = RESOURCE_GOLD;
    } else {
        city_mint->output_resource_id = RESOURCE_DENARII;
    }
    city_mint->data.industry.progress = 0;
    city_mint->data.industry.age_months = 0;
    city_mint->data.industry.average_production_per_month = 0;
    city_mint->data.industry.production_current_month = 0;
}

static void set_city_mint_conversion(const generic_button *button)
{
    resource_type resource = button->parameter1;
    if (building_get(data.city_mint_id)->output_resource_id != resource) {
        window_popup_dialog_show_confirmation(translation_for(TR_BUILDING_CITY_MINT_CHANGE_PRODUCTION),
            translation_for(TR_BUILDING_CITY_MINT_PROGRESS_WILL_BE_LOST), 0, city_mint_conversion_changed);
    }
}

int window_building_handle_mouse_city_mint(const mouse *m, building_info_context *c)
{
    if (!data.city_mint_id) {
        return 0;
    }
    if (generic_buttons_handle_mouse(m, c->x_offset + 16, c->y_offset + BLOCK_SIZE * c->height_blocks - 94,
        mint_conversion_buttons, 2, &data.focus_button_id)) {
        window_request_refresh();
        return 1;
    }
    return 0;
}

void window_building_industry_get_tooltip(building_info_context *c, int *translation)
{
    building_type type = building_get(c->building_id)->type;
    int needed_resources = building_get_raw_materials_for_workshop(0, type);
    int y_correction;
    if (type == BUILDING_CITY_MINT) {
        y_correction = 8;
    } else if (needed_resources > 0) {
        y_correction = 8 + needed_resources * 20;
    } else {
        y_correction = 0;
    }

    const mouse *m = mouse_get();
    if (m->x >= c->x_offset + 16 && m->x < c->width_blocks * BLOCK_SIZE + c->x_offset - 16 &&
        m->y >= c->y_offset + 60 + y_correction && m->y < c->y_offset + 86 + y_correction) {
        *translation = TR_BUILDING_WINDOW_INDUSTRY_EFFICIENCY_TOOLTIP;
    }
}
