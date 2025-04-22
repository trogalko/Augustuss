#include "resource.h"

#include "assets/assets.h"
#include "building/industry.h"
#include "building/type.h"
#include "core/image.h"
#include "core/image_group_editor.h"
#include "game/save_version.h"
#include "scenario/allowed_building.h"
#include "scenario/property.h"
#include "translation/translation.h"

#include <string.h>

#define RESOURCE_ALL (RESOURCE_MAX + RESOURCE_TOTAL_SPECIAL)

static const resource_type resource_mappings[][RESOURCE_ALL] = {
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_OLIVES, RESOURCE_VINES,
        RESOURCE_MEAT, RESOURCE_WINE, RESOURCE_OIL, RESOURCE_IRON, RESOURCE_TIMBER, RESOURCE_CLAY, RESOURCE_MARBLE,
        RESOURCE_WEAPONS, RESOURCE_FURNITURE, RESOURCE_POTTERY, RESOURCE_DENARII, RESOURCE_TROOPS
    },
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT,
        RESOURCE_CLAY, RESOURCE_TIMBER, RESOURCE_OLIVES, RESOURCE_VINES, RESOURCE_IRON, RESOURCE_MARBLE,
        RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE, RESOURCE_WEAPONS,
        RESOURCE_DENARII, RESOURCE_TROOPS
    },
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT, RESOURCE_FISH,
        RESOURCE_CLAY, RESOURCE_TIMBER, RESOURCE_OLIVES, RESOURCE_VINES, RESOURCE_IRON, RESOURCE_MARBLE,
        RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE, RESOURCE_WEAPONS,
        RESOURCE_DENARII, RESOURCE_TROOPS
    },
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT, RESOURCE_FISH,
        RESOURCE_CLAY, RESOURCE_TIMBER, RESOURCE_OLIVES, RESOURCE_VINES, RESOURCE_IRON, RESOURCE_MARBLE, RESOURCE_GOLD,
        RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE, RESOURCE_WEAPONS,
        RESOURCE_DENARII, RESOURCE_TROOPS
    },
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT, RESOURCE_FISH,
        RESOURCE_CLAY, RESOURCE_TIMBER, RESOURCE_OLIVES, RESOURCE_VINES, RESOURCE_IRON, RESOURCE_MARBLE, RESOURCE_GOLD, RESOURCE_SAND, RESOURCE_STONE,
        RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE, RESOURCE_WEAPONS, RESOURCE_CONCRETE, RESOURCE_BRICKS,
        RESOURCE_DENARII, RESOURCE_TROOPS
    }
};

static const resource_type legacy_inventory_mapping[LEGACY_INVENTORY_MAX] = {
    RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT,
    RESOURCE_WINE, RESOURCE_OIL, RESOURCE_FURNITURE, RESOURCE_POTTERY
};

static struct {
    resource_version_t version;
    const resource_type *resources;
    const resource_type *inventory;
    int total_resources;
    int total_food_resources;
    int special_resources;
    int joined_meat_and_fish;
} mapping;

static resource_data resource_info[RESOURCE_ALL] = {
    [RESOURCE_NONE]       = { .type = RESOURCE_NONE },
    [RESOURCE_WHEAT]      = { .type = RESOURCE_WHEAT,      .xml_attr_name = "wheat",       .flags = RESOURCE_FLAG_FOOD | RESOURCE_FLAG_INVENTORY, .industry = BUILDING_WHEAT_FARM,         .production_per_month = 160, .default_trade_price = {  28,  22 } },
    [RESOURCE_VEGETABLES] = { .type = RESOURCE_VEGETABLES, .xml_attr_name = "vegetables",  .flags = RESOURCE_FLAG_FOOD | RESOURCE_FLAG_INVENTORY, .industry = BUILDING_VEGETABLE_FARM,     .production_per_month = 80,  .default_trade_price = {  38,  30 } },
    [RESOURCE_FRUIT]      = { .type = RESOURCE_FRUIT,      .xml_attr_name = "fruit",       .flags = RESOURCE_FLAG_FOOD | RESOURCE_FLAG_INVENTORY, .industry = BUILDING_FRUIT_FARM,         .production_per_month = 80,  .default_trade_price = {  38,  30 } },
    [RESOURCE_MEAT]       = { .type = RESOURCE_MEAT,       .xml_attr_name = "meat",        .flags = RESOURCE_FLAG_FOOD | RESOURCE_FLAG_INVENTORY, .industry = BUILDING_PIG_FARM,           .production_per_month = 80,  .default_trade_price = {  44,  36 } },
    [RESOURCE_FISH]       = { .type = RESOURCE_FISH,       .xml_attr_name = "fish",        .flags = RESOURCE_FLAG_FOOD | RESOURCE_FLAG_INVENTORY, .industry = BUILDING_WHARF,              .production_per_month = 100, .default_trade_price = {  44,  36 } },
    [RESOURCE_CLAY]       = { .type = RESOURCE_CLAY,       .xml_attr_name = "clay",        .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_CLAY_PIT,           .production_per_month = 80,  .default_trade_price = {  40,  30 }, .warning = { WARNING_CLAY_NEEDED,   WARNING_BUILD_CLAY_PIT    } },
    [RESOURCE_TIMBER]     = { .type = RESOURCE_TIMBER,     .xml_attr_name = "timber|wood", .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_TIMBER_YARD,        .production_per_month = 80,  .default_trade_price = {  50,  35 }, .warning = { WARNING_TIMBER_NEEDED, WARNING_BUILD_TIMBER_YARD } },
    [RESOURCE_OLIVES]     = { .type = RESOURCE_OLIVES,     .xml_attr_name = "olives",      .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_OLIVE_FARM,         .production_per_month = 80,  .default_trade_price = {  42,  34 }, .warning = { WARNING_OLIVES_NEEDED, WARNING_BUILD_OLIVE_FARM  } },
    [RESOURCE_VINES]      = { .type = RESOURCE_VINES,      .xml_attr_name = "vines",       .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_VINES_FARM,         .production_per_month = 80,  .default_trade_price = {  44,  36 }, .warning = { WARNING_VINES_NEEDED,  WARNING_BUILD_VINES_FARM  } },
    [RESOURCE_IRON]       = { .type = RESOURCE_IRON,       .xml_attr_name = "iron",        .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_IRON_MINE,          .production_per_month = 80,  .default_trade_price = {  60,  40 }, .warning = { WARNING_IRON_NEEDED,   WARNING_BUILD_IRON_MINE   } },
    [RESOURCE_MARBLE]     = { .type = RESOURCE_MARBLE,     .xml_attr_name = "marble",      .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_MARBLE_QUARRY,      .production_per_month = 40,  .default_trade_price = { 200, 140 } },
    [RESOURCE_GOLD]       = { .type = RESOURCE_GOLD,       .xml_attr_name = "gold",        .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_GOLD_MINE,          .production_per_month = 20,  .default_trade_price = { 350, 250 }, .warning = { WARNING_GOLD_NEEDED,   WARNING_BUILD_GOLD_MINE   } },
    [RESOURCE_SAND]       = { .type = RESOURCE_SAND,       .xml_attr_name = "sand",        .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_SAND_PIT,           .production_per_month = 120, .default_trade_price = {  30,  24 }, .warning = { WARNING_SAND_NEEDED,   WARNING_BUILD_SAND_PIT    } },
    [RESOURCE_STONE]      = { .type = RESOURCE_STONE,      .xml_attr_name = "stone",       .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_STONE_QUARRY,       .production_per_month = 80,  .default_trade_price = {  40,  30 }, .warning = { WARNING_STONE_NEEDED,  WARNING_BUILD_STONE_MINE  } },
    [RESOURCE_POTTERY]    = { .type = RESOURCE_POTTERY,    .xml_attr_name = "pottery",     .flags = RESOURCE_FLAG_INVENTORY,                      .industry = BUILDING_POTTERY_WORKSHOP,   .production_per_month = 40,  .default_trade_price = { 180, 140 } },
    [RESOURCE_FURNITURE]  = { .type = RESOURCE_FURNITURE,  .xml_attr_name = "furniture",   .flags = RESOURCE_FLAG_INVENTORY,                      .industry = BUILDING_FURNITURE_WORKSHOP, .production_per_month = 40,  .default_trade_price = { 200, 150 } },
    [RESOURCE_OIL]        = { .type = RESOURCE_OIL,        .xml_attr_name = "oil",         .flags = RESOURCE_FLAG_INVENTORY,                      .industry = BUILDING_OIL_WORKSHOP,       .production_per_month = 40,  .default_trade_price = { 180, 140 } },
    [RESOURCE_WINE]       = { .type = RESOURCE_WINE,       .xml_attr_name = "wine",        .flags = RESOURCE_FLAG_INVENTORY,                      .industry = BUILDING_WINE_WORKSHOP,      .production_per_month = 40,  .default_trade_price = { 215, 160 } },
    [RESOURCE_WEAPONS]    = { .type = RESOURCE_WEAPONS,    .xml_attr_name = "weapons",     .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_WEAPONS_WORKSHOP,   .production_per_month = 40,  .default_trade_price = { 250, 180 } },
    [RESOURCE_CONCRETE]   = { .type = RESOURCE_CONCRETE,   .xml_attr_name = "concrete",    .flags = RESOURCE_FLAG_NONE,                           .industry = BUILDING_CONCRETE_MAKER,     .production_per_month = 120 },
    [RESOURCE_BRICKS]     = { .type = RESOURCE_BRICKS,     .xml_attr_name = "bricks",      .flags = RESOURCE_FLAG_STORABLE,                       .industry = BUILDING_BRICKWORKS,         .production_per_month = 60,  .default_trade_price = { 200, 150 } },
    [RESOURCE_DENARII]    = { .type = RESOURCE_DENARII,    .industry = BUILDING_CITY_MINT, .production_per_month = 200 },
    [RESOURCE_TROOPS]     = { .type = RESOURCE_TROOPS  }
};

static const resource_supply_chain SUPPLY_CHAIN[RESOURCE_SUPPLY_CHAIN_MAX_SIZE] = {
    { .raw_amount = 100, .raw_material = RESOURCE_CLAY,   .good = RESOURCE_POTTERY   },
    { .raw_amount = 100, .raw_material = RESOURCE_TIMBER, .good = RESOURCE_FURNITURE },
    { .raw_amount = 100, .raw_material = RESOURCE_OLIVES, .good = RESOURCE_OIL       },
    { .raw_amount = 100, .raw_material = RESOURCE_VINES,  .good = RESOURCE_WINE      },
    { .raw_amount = 100, .raw_material = RESOURCE_IRON,   .good = RESOURCE_WEAPONS   },
    { .raw_amount =  20, .raw_material = RESOURCE_GOLD,   .good = RESOURCE_DENARII   },
    { .raw_amount =  50, .raw_material = RESOURCE_SAND,   .good = RESOURCE_CONCRETE  },
    { .raw_amount =  50, .raw_material = RESOURCE_SAND,   .good = RESOURCE_BRICKS    },
    { .raw_amount =  50, .raw_material = RESOURCE_CLAY,   .good = RESOURCE_BRICKS    },
};

int resource_is_food(resource_type resource)
{
    return (resource_info[resource].flags & RESOURCE_FLAG_FOOD) == RESOURCE_FLAG_FOOD;
}

int resource_is_raw_material(resource_type resource)
{
    return resource != RESOURCE_NONE && !resource_is_food(resource) &&
        resource_get_supply_chain_for_good(0, resource) == 0;
}

int resource_is_inventory(resource_type resource)
{
    return (resource_info[resource].flags & RESOURCE_FLAG_INVENTORY) == RESOURCE_FLAG_INVENTORY;
}

int resource_is_storable(resource_type resource)
{
    return (resource_info[resource].flags & RESOURCE_FLAG_STORABLE) == RESOURCE_FLAG_STORABLE;
}

resource_type resource_get_from_industry(building_type industry)
{
    if (industry == resource_info[RESOURCE_DENARII].industry) {
        return RESOURCE_DENARII;
    }
    for (resource_type resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (resource_info[resource].industry == industry) {
            return resource;
        }
    }
    return RESOURCE_NONE;
}

int resource_get_supply_chain_for_good(resource_supply_chain *chain, resource_type good)
{
    int current_position = 0;
    for (int i = 0; i < RESOURCE_SUPPLY_CHAIN_MAX_SIZE; i++) {
        if (SUPPLY_CHAIN[i].good == good) {
            if (chain) {
                chain[current_position] = SUPPLY_CHAIN[i];
            }
            current_position++;
        }
    }
    return current_position;
}

int resource_get_supply_chain_for_raw_material(resource_supply_chain *chain, resource_type raw_material)
{
    int current_position = 0;
    for (int i = 0; i < RESOURCE_SUPPLY_CHAIN_MAX_SIZE; i++) {
        if (SUPPLY_CHAIN[i].raw_material == raw_material) {
            if (chain) {
                chain[current_position] = SUPPLY_CHAIN[i];
            }
            current_position++;
        }
    }
    return current_position;
}

void resource_init(void)
{
    int food_index = 0;
    int good_index = 0;

    for (int i = RESOURCE_MIN; i < RESOURCE_MAX_LEGACY; i++) {
        resource_data *info = &resource_info[resource_mappings[0][i]];
        info->text = lang_get_string(23, i);
        info->image.cart.single_load = image_group(GROUP_FIGURE_CARTPUSHER_CART) + 8 * i;
        if (resource_is_food(info->type)) {
            info->image.cart.multiple_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 8 * food_index;
            info->image.cart.eight_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 32 + 8 * (food_index + 1);
            food_index++;
        } else {
            info->image.cart.multiple_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_RESOURCE) + 8 * good_index;
            info->image.cart.eight_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_RESOURCE) + 8 * good_index;
            good_index++;
        }
        info->image.empire = image_group(GROUP_EMPIRE_RESOURCES) + i;
        info->image.icon = image_group(GROUP_RESOURCE_ICONS) + i;
        info->image.storage = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) + 4 * (i - 1);
        info->image.editor.icon = image_group(GROUP_EDITOR_RESOURCE_ICONS) + i;
        info->image.editor.empire = image_group(GROUP_EDITOR_EMPIRE_RESOURCES) + i;
    }

    resource_info[RESOURCE_FISH].text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_FISH);
    resource_info[RESOURCE_FISH].image.cart.single_load = image_group(GROUP_FIGURE_CARTPUSHER_CART) + 696;
    resource_info[RESOURCE_FISH].image.cart.multiple_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 32;
    resource_info[RESOURCE_FISH].image.cart.eight_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 72;
    resource_info[RESOURCE_FISH].image.storage = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) + 60;
    resource_info[RESOURCE_FISH].image.icon = image_group(GROUP_RESOURCE_ICONS) + 17;
    resource_info[RESOURCE_FISH].image.empire = image_group(GROUP_EMPIRE_RESOURCES) + 17;
    resource_info[RESOURCE_FISH].image.editor.icon = image_group(GROUP_EDITOR_RESOURCE_ICONS) + 17;
    resource_info[RESOURCE_FISH].image.editor.empire = image_group(GROUP_EDITOR_EMPIRE_RESOURCES) + 17;

    resource_info[RESOURCE_GOLD].text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_GOLD);
    resource_info[RESOURCE_GOLD].image.cart.single_load = assets_get_image_id("Industry", "Gold_Cart_NE");
    resource_info[RESOURCE_GOLD].image.cart.multiple_loads = assets_get_image_id("Industry", "Gold_Cart_Getting_NE");
    resource_info[RESOURCE_GOLD].image.cart.eight_loads = assets_get_image_id("Industry", "Gold_Cart_Getting_NE");
    resource_info[RESOURCE_GOLD].image.storage = assets_get_image_id("Industry", "Warehouse_Gold_01");
    resource_info[RESOURCE_GOLD].image.icon = assets_get_image_id("UI", "Panelling_Gold_01");
    resource_info[RESOURCE_GOLD].image.empire = assets_get_image_id("UI", "Panelling_Gold_02");
    resource_info[RESOURCE_GOLD].image.editor.icon = assets_get_image_id("UI", "Panelling_Gold_01");
    resource_info[RESOURCE_GOLD].image.editor.empire = assets_get_image_id("UI", "Panelling_Gold_02");

    resource_info[RESOURCE_STONE].text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_STONE);
    resource_info[RESOURCE_STONE].image.cart.single_load = assets_get_image_id("Industry", "Stone_Cart_NE");
    resource_info[RESOURCE_STONE].image.cart.multiple_loads = assets_get_image_id("Industry", "Stone_Cart_Getting_NE");
    resource_info[RESOURCE_STONE].image.cart.eight_loads = assets_get_image_id("Industry", "Stone_Cart_Getting_NE");
    resource_info[RESOURCE_STONE].image.storage = assets_get_image_id("Industry", "Warehouse_Stone_01");
    resource_info[RESOURCE_STONE].image.icon = assets_get_image_id("UI", "Panelling_Stone_01");
    resource_info[RESOURCE_STONE].image.empire = assets_get_image_id("UI", "Panelling_Stone_02");
    resource_info[RESOURCE_STONE].image.editor.icon = assets_get_image_id("UI", "Panelling_Stone_01");
    resource_info[RESOURCE_STONE].image.editor.empire = assets_get_image_id("UI", "Panelling_Stone_02");
    
    resource_info[RESOURCE_SAND].text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_SAND);
    resource_info[RESOURCE_SAND].image.cart.single_load = assets_get_image_id("Industry", "Sand_Cart_NE");
    resource_info[RESOURCE_SAND].image.cart.multiple_loads = assets_get_image_id("Industry", "Sand_Cart_Getting_NE");
    resource_info[RESOURCE_SAND].image.cart.eight_loads = assets_get_image_id("Industry", "Sand_Cart_Getting_NE");
    resource_info[RESOURCE_SAND].image.storage = assets_get_image_id("Industry", "Warehouse_Sand_01");
    resource_info[RESOURCE_SAND].image.icon = assets_get_image_id("UI", "Panelling_Sand_01");
    resource_info[RESOURCE_SAND].image.empire = assets_get_image_id("UI", "Panelling_Sand_02");
    resource_info[RESOURCE_SAND].image.editor.icon = assets_get_image_id("UI", "Panelling_Sand_01");
    resource_info[RESOURCE_SAND].image.editor.empire = assets_get_image_id("UI", "Panelling_Sand_02");

    resource_info[RESOURCE_CONCRETE].text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_CONCRETE);
    resource_info[RESOURCE_CONCRETE].image.cart.single_load = assets_get_image_id("Industry", "Concrete_Cart_NE");
    resource_info[RESOURCE_CONCRETE].image.cart.multiple_loads = assets_get_image_id("Industry", "Concrete_Cart_Getting_NE");
    resource_info[RESOURCE_CONCRETE].image.cart.eight_loads = assets_get_image_id("Industry", "Concrete_Cart_Getting_NE");
    resource_info[RESOURCE_CONCRETE].image.icon = assets_get_image_id("UI", "Panelling_Concrete_01");
    resource_info[RESOURCE_CONCRETE].image.empire = assets_get_image_id("UI", "Panelling_Concrete_02");
    resource_info[RESOURCE_CONCRETE].image.editor.icon = assets_get_image_id("UI", "Panelling_Concrete_01");
    resource_info[RESOURCE_CONCRETE].image.editor.empire = assets_get_image_id("UI", "Panelling_Concrete_02");

    resource_info[RESOURCE_BRICKS].text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_BRICKS);
    resource_info[RESOURCE_BRICKS].image.cart.single_load = assets_get_image_id("Industry", "Brick_Cart_NE");
    resource_info[RESOURCE_BRICKS].image.cart.multiple_loads = assets_get_image_id("Industry", "Brick_Cart_Getting_NE");
    resource_info[RESOURCE_BRICKS].image.cart.eight_loads = assets_get_image_id("Industry", "Brick_Cart_Getting_NE");
    resource_info[RESOURCE_BRICKS].image.storage = assets_get_image_id("Industry", "Warehouse_Bricks_01");
    resource_info[RESOURCE_BRICKS].image.icon = assets_get_image_id("UI", "Panelling_Bricks_01");
    resource_info[RESOURCE_BRICKS].image.empire = assets_get_image_id("UI", "Panelling_Bricks_02");
    resource_info[RESOURCE_BRICKS].image.editor.icon = assets_get_image_id("UI", "Panelling_Bricks_01");
    resource_info[RESOURCE_BRICKS].image.editor.empire = assets_get_image_id("UI", "Panelling_Bricks_02");
    
    resource_info[RESOURCE_NONE].text = lang_get_string(23, 0);

    resource_info[RESOURCE_DENARII].image.icon = image_group(GROUP_RESOURCE_ICONS) + 16;
    resource_info[RESOURCE_DENARII].image.editor.icon = image_group(GROUP_EDITOR_RESOURCE_ICONS) + 16;
    resource_info[RESOURCE_DENARII].text = lang_get_string(23, 16);

    resource_info[RESOURCE_TROOPS].text = lang_get_string(23, 17);
}

const resource_data *resource_get_data(resource_type resource)
{
    return &resource_info[resource];
}

void resource_set_mapping(resource_version_t version)
{
    mapping.version = version;
    switch (version) {
        case RESOURCE_ORIGINAL_VERSION:
            mapping.resources = resource_mappings[0];
            mapping.inventory = legacy_inventory_mapping;
            mapping.total_resources = RESOURCE_MAX_LEGACY;
            mapping.total_food_resources = RESOURCE_MAX_FOOD_LEGACY;
            mapping.joined_meat_and_fish = 1;
            break;
        case RESOURCE_DYNAMIC_VERSION:
            mapping.resources = resource_mappings[0];
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX_LEGACY;
            mapping.total_food_resources = RESOURCE_MAX_FOOD_LEGACY;
            mapping.joined_meat_and_fish = 1;
            break;
        case RESOURCE_REORDERED_VERSION:
            mapping.resources = resource_mappings[1];
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX_LEGACY;
            mapping.total_food_resources = RESOURCE_MAX_FOOD_REORDERED;
            mapping.joined_meat_and_fish = 1;
            break;
        case RESOURCE_SEPARATE_FISH_AND_MEAT_VERSION:
            mapping.resources = resource_mappings[2];
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX_WITH_FISH;
            mapping.total_food_resources = RESOURCE_MAX_FOOD_WITH_FISH;
            mapping.joined_meat_and_fish = 0;
            break;
        case RESOURCE_HAS_GOLD_VERSION:
            mapping.resources = resource_mappings[3];
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX_WITH_GOLD;
            mapping.total_food_resources = RESOURCE_MAX_FOOD;
            mapping.joined_meat_and_fish = 0;
            break;
        case RESOURCE_HAS_NEW_MONUMENT_ELEMENTS:
        default:
            mapping.resources = resource_mappings[4];
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX;
            mapping.total_food_resources = RESOURCE_MAX_FOOD;
            mapping.joined_meat_and_fish = 0;
            break;
    }
}

resource_type resource_map_legacy_inventory(int id)
{
    resource_type resource = mapping.inventory ? mapping.inventory[id] : id;
    if (mapping.joined_meat_and_fish && resource == RESOURCE_MEAT && scenario_allowed_building(BUILDING_WHARF)) {
        return RESOURCE_FISH;
    }
    return resource;
}

resource_version_t resource_mapping_get_version(void)
{
    return mapping.version;
}

int resource_production_per_month(resource_type resource)
{
    int production = resource_info[resource].production_per_month;
    if (resource == RESOURCE_WHEAT && scenario_property_climate() == CLIMATE_NORTHERN) {
        production /= 2;
    }
    return production;
}

resource_type resource_remap(int id)
{
    resource_type resource = mapping.resources ? mapping.resources[id] : id;
    if (mapping.joined_meat_and_fish && resource == RESOURCE_MEAT && scenario_allowed_building(BUILDING_WHARF)) {
        return RESOURCE_FISH;
    }
    return resource;
}

int resource_total_mapped(void)
{
    return mapping.total_resources;
}

int resource_total_food_mapped(void)
{
    return mapping.total_food_resources;
}
