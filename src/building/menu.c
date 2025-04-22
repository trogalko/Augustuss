#include "menu.h"

#include "building/monument.h"
#include "building/properties.h"
#include "city/buildings.h"
#include "core/config.h"
#include "empire/city.h"
#include "game/tutorial.h"
#include "scenario/allowed_building.h"
#include "scenario/property.h"

#define BUILD_MENU_ITEM_MAX 30

#define MAIN_MENU_NUM_ENTRIES 11

static const building_type MENU_BUILDING_TYPE[BUILD_MENU_MAX][BUILD_MENU_ITEM_MAX] = {
    {BUILDING_HOUSE_VACANT_LOT, 0},
    {BUILDING_CLEAR_LAND, 0},
    {BUILDING_ROAD, BUILDING_HIGHWAY, 0},
    {BUILDING_DRAGGABLE_RESERVOIR, BUILDING_AQUEDUCT, BUILDING_FOUNTAIN, BUILDING_WELL, 0},
    {BUILDING_BARBER, BUILDING_BATHHOUSE, BUILDING_DOCTOR, BUILDING_HOSPITAL, 0},
    {BUILDING_MENU_SMALL_TEMPLES, BUILDING_MENU_LARGE_TEMPLES, BUILDING_MENU_GRAND_TEMPLES, BUILDING_MENU_SHRINES, BUILDING_LARARIUM,
        BUILDING_ORACLE, BUILDING_SMALL_MAUSOLEUM, BUILDING_LARGE_MAUSOLEUM, BUILDING_NYMPHAEUM, 0},
    {BUILDING_SCHOOL, BUILDING_ACADEMY, BUILDING_LIBRARY, BUILDING_MISSION_POST, 0},
    {BUILDING_THEATER, BUILDING_TAVERN, BUILDING_AMPHITHEATER, BUILDING_ARENA, BUILDING_COLOSSEUM, BUILDING_HIPPODROME,
        BUILDING_GLADIATOR_SCHOOL, BUILDING_LION_HOUSE, BUILDING_ACTOR_COLONY, BUILDING_CHARIOT_MAKER, 0},
    {BUILDING_MENU_STATUES, BUILDING_MENU_TREES, BUILDING_MENU_PARKS, BUILDING_MENU_PATHS, BUILDING_MENU_GOV_RES,
        BUILDING_MENU_GARDENS, BUILDING_PLAZA, BUILDING_ROADBLOCK, BUILDING_FORUM, BUILDING_SENATE, BUILDING_CITY_MINT,
        BUILDING_TRIUMPHAL_ARCH, 0},
    {BUILDING_ENGINEERS_POST, BUILDING_LOW_BRIDGE, BUILDING_SHIP_BRIDGE, BUILDING_SHIPYARD, BUILDING_DOCK,
        BUILDING_WHARF, BUILDING_WORKCAMP, BUILDING_ARCHITECT_GUILD, BUILDING_LIGHTHOUSE, 0},
    {BUILDING_WALL, BUILDING_TOWER, BUILDING_GATEHOUSE, BUILDING_PALISADE, BUILDING_PREFECTURE,
        BUILDING_FORT, BUILDING_MILITARY_ACADEMY, BUILDING_BARRACKS, BUILDING_MESS_HALL, BUILDING_ARMOURY, BUILDING_WATCHTOWER, 0},
    {BUILDING_MENU_FARMS, BUILDING_MENU_RAW_MATERIALS, BUILDING_MENU_WORKSHOPS,
        BUILDING_MARKET, BUILDING_GRANARY, BUILDING_WAREHOUSE, BUILDING_CARAVANSERAI, BUILDING_DEPOT, 0},
    {BUILDING_WHEAT_FARM, BUILDING_VEGETABLE_FARM, BUILDING_FRUIT_FARM,
        BUILDING_OLIVE_FARM, BUILDING_VINES_FARM, BUILDING_PIG_FARM, 0},
    {BUILDING_CLAY_PIT, BUILDING_MARBLE_QUARRY, BUILDING_IRON_MINE, BUILDING_TIMBER_YARD, BUILDING_GOLD_MINE,
        BUILDING_STONE_QUARRY, BUILDING_SAND_PIT, 0},
    {BUILDING_WINE_WORKSHOP, BUILDING_OIL_WORKSHOP, BUILDING_WEAPONS_WORKSHOP,
        BUILDING_FURNITURE_WORKSHOP, BUILDING_POTTERY_WORKSHOP, BUILDING_BRICKWORKS, BUILDING_CONCRETE_MAKER, 0},
    {BUILDING_MENU_SMALL_TEMPLES, BUILDING_SMALL_TEMPLE_CERES, BUILDING_SMALL_TEMPLE_NEPTUNE,
        BUILDING_SMALL_TEMPLE_MERCURY, BUILDING_SMALL_TEMPLE_MARS, BUILDING_SMALL_TEMPLE_VENUS, 0},
    {BUILDING_MENU_LARGE_TEMPLES, BUILDING_LARGE_TEMPLE_CERES, BUILDING_LARGE_TEMPLE_NEPTUNE,
        BUILDING_LARGE_TEMPLE_MERCURY, BUILDING_LARGE_TEMPLE_MARS, BUILDING_LARGE_TEMPLE_VENUS, 0},
    {BUILDING_FORT_LEGIONARIES, BUILDING_FORT_JAVELIN, BUILDING_FORT_MOUNTED, BUILDING_FORT_AUXILIA_INFANTRY, BUILDING_FORT_ARCHERS, 0},
    {BUILDING_OBELISK, BUILDING_DECORATIVE_COLUMN, BUILDING_COLONNADE, BUILDING_HEDGE_LIGHT, BUILDING_HEDGE_DARK,
        BUILDING_LOOPED_GARDEN_WALL, BUILDING_ROOFED_GARDEN_WALL, BUILDING_PANELLED_GARDEN_WALL, BUILDING_PAVILION_BLUE, BUILDING_SMALL_POND,
        BUILDING_LARGE_POND, 0},
    {BUILDING_MENU_TREES, BUILDING_PINE_TREE, BUILDING_FIR_TREE, BUILDING_OAK_TREE, BUILDING_ELM_TREE, BUILDING_FIG_TREE, BUILDING_PLUM_TREE,
        BUILDING_PALM_TREE, BUILDING_DATE_TREE, 0},
    {BUILDING_MENU_PATHS, BUILDING_GARDEN_PATH, BUILDING_PINE_PATH , BUILDING_FIR_PATH, BUILDING_OAK_PATH, BUILDING_ELM_PATH,
        BUILDING_FIG_PATH, BUILDING_PLUM_PATH, BUILDING_PALM_PATH, BUILDING_DATE_PATH, 0},
    {BUILDING_PANTHEON, BUILDING_GRAND_TEMPLE_CERES, BUILDING_GRAND_TEMPLE_NEPTUNE, BUILDING_GRAND_TEMPLE_MERCURY,
        BUILDING_GRAND_TEMPLE_MARS, BUILDING_GRAND_TEMPLE_VENUS, 0},
    {BUILDING_SMALL_STATUE, BUILDING_GODDESS_STATUE, BUILDING_SENATOR_STATUE, BUILDING_GLADIATOR_STATUE,
        BUILDING_MEDIUM_STATUE, BUILDING_LEGION_STATUE, BUILDING_LARGE_STATUE, BUILDING_HORSE_STATUE, 0},
    {BUILDING_GOVERNORS_HOUSE, BUILDING_GOVERNORS_VILLA, BUILDING_GOVERNORS_PALACE, 0},
    {BUILDING_MENU_SHRINES, BUILDING_SHRINE_CERES, BUILDING_SHRINE_NEPTUNE, BUILDING_SHRINE_MERCURY, BUILDING_SHRINE_MARS, BUILDING_SHRINE_VENUS, 0},
    {BUILDING_MENU_GARDENS, BUILDING_GARDENS, BUILDING_OVERGROWN_GARDENS, 0}
};

static const building_type BUILD_MENU_TYPE_TO_BUILDING_TYPE[BUILD_MENU_MAX] = {
    [BUILD_MENU_FARMS]         = BUILDING_MENU_FARMS,
    [BUILD_MENU_RAW_MATERIALS] = BUILDING_MENU_RAW_MATERIALS,
    [BUILD_MENU_WORKSHOPS]     = BUILDING_MENU_WORKSHOPS,
    [BUILD_MENU_SMALL_TEMPLES] = BUILDING_MENU_SMALL_TEMPLES,
    [BUILD_MENU_LARGE_TEMPLES] = BUILDING_MENU_LARGE_TEMPLES,
    [BUILD_MENU_FORTS]         = BUILDING_FORT,
    [BUILD_MENU_GRAND_TEMPLES] = BUILDING_MENU_GRAND_TEMPLES,
    [BUILD_MENU_PARKS]         = BUILDING_MENU_PARKS,
    [BUILD_MENU_TREES]         = BUILDING_MENU_TREES,
    [BUILD_MENU_PATHS]         = BUILDING_MENU_PATHS,
    [BUILD_MENU_GOV_RES]       = BUILDING_MENU_GOV_RES,
    [BUILD_MENU_STATUES]       = BUILDING_MENU_STATUES,
    [BUILD_MENU_SHRINES]       = BUILDING_MENU_SHRINES,
    [BUILD_MENU_GARDENS]       = BUILDING_MENU_GARDENS,
};

static int menu_enabled[BUILD_MENU_MAX][BUILD_MENU_ITEM_MAX];

static int changed = 1;

void building_menu_enable_all(void)
{
    for (int sub = 0; sub < BUILD_MENU_MAX; sub++) {
        for (int item = 0; item < BUILD_MENU_ITEM_MAX; item++) {
            menu_enabled[sub][item] = 1;
        }
    }
}

static void enable_house(int *enabled, building_type menu_building_type)
{
    if (menu_building_type >= BUILDING_HOUSE_VACANT_LOT && menu_building_type <= BUILDING_HOUSE_LUXURY_PALACE) {
        *enabled = 1;
    }
}

static void enable_clear(int *enabled, building_type menu_building_type)
{
    if (menu_building_type == BUILDING_CLEAR_LAND) {
        *enabled = 1;
    }
}

static void enable_cycling_temples_if_allowed(building_type type)
{
    int sub = (type == BUILDING_MENU_SMALL_TEMPLES) ? BUILD_MENU_SMALL_TEMPLES : BUILD_MENU_LARGE_TEMPLES;
    menu_enabled[sub][0] = 0;
    if (building_menu_count_items(sub) > 1) {
        menu_enabled[sub][0] = 1;
    }
}

static int is_building_type_allowed(building_type type);

static int can_get_required_resource(building_type type)
{
    switch (type) {
        case BUILDING_SHIPYARD:
            return empire_can_produce_resource_naturally(RESOURCE_FISH);
        case BUILDING_TAVERN:
            return empire_can_produce_resource_potentially(RESOURCE_WINE) ||
                empire_can_import_resource_potentially(RESOURCE_WINE);
        case BUILDING_LIGHTHOUSE:
            return (empire_can_produce_resource_potentially(RESOURCE_TIMBER) ||
                empire_can_import_resource_potentially(RESOURCE_TIMBER)) &&
                building_monument_has_required_resources_to_build(type);
        case BUILDING_CITY_MINT:
            return is_building_type_allowed(BUILDING_SENATE) &&
                building_monument_has_required_resources_to_build(type);
        default:
            return building_monument_has_required_resources_to_build(type);
    }
}

static int is_building_type_allowed(building_type type)
{
    return scenario_allowed_building(type) && can_get_required_resource(type);
}

static void enable_if_allowed(int *enabled, building_type menu_building_type, building_type type)
{
    if (menu_building_type != type) {
        return;
    }
    if (type == BUILDING_MENU_SMALL_TEMPLES || type == BUILDING_MENU_LARGE_TEMPLES) {
        *enabled = 1;
        enable_cycling_temples_if_allowed(type);
    } else {
        *enabled = is_building_type_allowed(type);
    }
}

static void disable_raw(int *enabled, building_type menu_building_type, building_type type, int resource)
{
    if (type != menu_building_type) {
        return;
    }
    if (!empire_can_produce_resource_naturally(resource)) {
        *enabled = 0;
    }
    if (scenario_is_tutorial_2() && resource == RESOURCE_SAND) {
        *enabled = 0;
    }
}

static void disable_finished(int *enabled, building_type menu_building_type, building_type type, int resource)
{
    if (type != menu_building_type) {
        return;
    }
    if (!empire_can_produce_resource_potentially(resource)) {
        *enabled = 0;
    }
    if (scenario_is_tutorial_2() && (resource != RESOURCE_POTTERY && resource != RESOURCE_WEAPONS)) {
        *enabled = 0;
    }
}

static void disable_if_no_enabled_submenu_items(int *enabled, int submenu)
{
    for (int item = 0; item < BUILD_MENU_ITEM_MAX && MENU_BUILDING_TYPE[submenu][item]; item++) {
        if (BUILD_MENU_TYPE_TO_BUILDING_TYPE[submenu] == MENU_BUILDING_TYPE[submenu][item]) {
            continue;
        }
        if (is_building_type_allowed(MENU_BUILDING_TYPE[submenu][item])) {
            return;
        }
    }
    *enabled = 0;
}

static void enable_normal(int *enabled, building_type type)
{
    for (building_type current_type = BUILDING_NONE; current_type < BUILDING_TYPE_MAX; current_type++) {
        enable_if_allowed(enabled, type, current_type);
    }

    if (type == BUILDING_TRIUMPHAL_ARCH && !city_buildings_triumphal_arch_available()) {
        *enabled = 0;
    }
}

static void enable_tutorial1_start(int *enabled, building_type type)
{
    enable_house(enabled, type);
    enable_clear(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_WELL);
    enable_if_allowed(enabled, type, BUILDING_ROAD);
}

static void enable_tutorial1_after_fire(int *enabled, building_type type)
{
    enable_tutorial1_start(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_PREFECTURE);
    enable_if_allowed(enabled, type, BUILDING_MARKET);
}

static void enable_tutorial1_after_collapse(int *enabled, building_type type)
{
    enable_tutorial1_after_fire(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_ENGINEERS_POST);
    enable_if_allowed(enabled, type, BUILDING_SENATE);
    enable_if_allowed(enabled, type, BUILDING_ROADBLOCK);
}

static void enable_tutorial1_after_senate(int *enabled, building_type type)
{
    enable_tutorial1_after_collapse(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_MENU_SMALL_TEMPLES);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_CERES);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_NEPTUNE);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_MERCURY);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_MARS);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_VENUS);
}

static void enable_tutorial2_start(int *enabled, building_type type)
{
    enable_house(enabled, type);
    enable_clear(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_WELL);
    enable_if_allowed(enabled, type, BUILDING_ROAD);
    enable_if_allowed(enabled, type, BUILDING_PREFECTURE);
    enable_if_allowed(enabled, type, BUILDING_ENGINEERS_POST);
    enable_if_allowed(enabled, type, BUILDING_SENATE);
    enable_if_allowed(enabled, type, BUILDING_ROADBLOCK);
    enable_if_allowed(enabled, type, BUILDING_MARKET);
    enable_if_allowed(enabled, type, BUILDING_GRANARY);
    enable_if_allowed(enabled, type, BUILDING_MENU_FARMS);
    enable_if_allowed(enabled, type, BUILDING_WHEAT_FARM);
    enable_if_allowed(enabled, type, BUILDING_VEGETABLE_FARM);
    enable_if_allowed(enabled, type, BUILDING_FRUIT_FARM);
    enable_if_allowed(enabled, type, BUILDING_PIG_FARM);
    enable_if_allowed(enabled, type, BUILDING_MENU_SMALL_TEMPLES);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_CERES);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_NEPTUNE);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_MERCURY);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_MARS);
    enable_if_allowed(enabled, type, BUILDING_SMALL_TEMPLE_VENUS);
}

static void enable_tutorial2_up_to_250(int *enabled, building_type type)
{
    enable_tutorial2_start(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_DRAGGABLE_RESERVOIR);
    enable_if_allowed(enabled, type, BUILDING_AQUEDUCT);
    enable_if_allowed(enabled, type, BUILDING_FOUNTAIN);
}

static void enable_tutorial2_up_to_450(int *enabled, building_type type)
{
    enable_tutorial2_up_to_250(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_MENU_GARDENS);
    enable_if_allowed(enabled, type, BUILDING_GARDENS);
    enable_if_allowed(enabled, type, BUILDING_OVERGROWN_GARDENS);
    enable_if_allowed(enabled, type, BUILDING_ACTOR_COLONY);
    enable_if_allowed(enabled, type, BUILDING_THEATER);
    enable_if_allowed(enabled, type, BUILDING_BATHHOUSE);
    enable_if_allowed(enabled, type, BUILDING_SCHOOL);
}

static void enable_tutorial2_after_450(int *enabled, building_type type)
{
    enable_tutorial2_up_to_450(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_MENU_RAW_MATERIALS);
    enable_if_allowed(enabled, type, BUILDING_CLAY_PIT);
    enable_if_allowed(enabled, type, BUILDING_MARBLE_QUARRY);
    enable_if_allowed(enabled, type, BUILDING_IRON_MINE);
    enable_if_allowed(enabled, type, BUILDING_TIMBER_YARD);
    enable_if_allowed(enabled, type, BUILDING_GOLD_MINE);
    enable_if_allowed(enabled, type, BUILDING_STONE_QUARRY);
    enable_if_allowed(enabled, type, BUILDING_SAND_PIT);
    enable_if_allowed(enabled, type, BUILDING_MENU_WORKSHOPS);
    enable_if_allowed(enabled, type, BUILDING_WINE_WORKSHOP);
    enable_if_allowed(enabled, type, BUILDING_OIL_WORKSHOP);
    enable_if_allowed(enabled, type, BUILDING_WEAPONS_WORKSHOP);
    enable_if_allowed(enabled, type, BUILDING_FURNITURE_WORKSHOP);
    enable_if_allowed(enabled, type, BUILDING_POTTERY_WORKSHOP);
    enable_if_allowed(enabled, type, BUILDING_BRICKWORKS);
    enable_if_allowed(enabled, type, BUILDING_CONCRETE_MAKER);
    enable_if_allowed(enabled, type, BUILDING_WAREHOUSE);
    enable_if_allowed(enabled, type, BUILDING_FORUM);
    enable_if_allowed(enabled, type, BUILDING_AMPHITHEATER);
    enable_if_allowed(enabled, type, BUILDING_GLADIATOR_SCHOOL);
}

static void disable_resources(int *enabled, building_type type)
{
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (resource_is_food(r) || resource_is_raw_material(r)) {
            disable_raw(enabled, type, resource_get_data(r)->industry, r);
        } else {
            disable_finished(enabled, type, resource_get_data(r)->industry, r);
        }
    }
}

void building_menu_update(void)
{
    tutorial_build_buttons tutorial_buttons = tutorial_get_build_buttons();
    for (int sub = 0; sub < BUILD_MENU_MAX; sub++) {
        for (int item = 0; item < BUILD_MENU_ITEM_MAX; item++) {
            building_type type = MENU_BUILDING_TYPE[sub][item];
            int *menu_item = &menu_enabled[sub][item];
            // first 12 items, parks and grand temples always disabled at the start
            if (sub <= MAIN_MENU_NUM_ENTRIES || sub == 18 || sub == 21) {
                *menu_item = 0;
            } else {
                *menu_item = 1;
            }
            switch (tutorial_buttons) {
                case TUT1_BUILD_START:
                    enable_tutorial1_start(menu_item, type);
                    break;
                case TUT1_BUILD_AFTER_FIRE:
                    enable_tutorial1_after_fire(menu_item, type);
                    break;
                case TUT1_BUILD_AFTER_COLLAPSE:
                    enable_tutorial1_after_collapse(menu_item, type);
                    break;
                case TUT1_BUILD_AFTER_SENATE:
                    enable_tutorial1_after_senate(menu_item, type);
                    break;
                case TUT2_BUILD_START:
                    enable_tutorial2_start(menu_item, type);
                    break;
                case TUT2_BUILD_UP_TO_250:
                    enable_tutorial2_up_to_250(menu_item, type);
                    break;
                case TUT2_BUILD_UP_TO_450:
                    enable_tutorial2_up_to_450(menu_item, type);
                    break;
                case TUT2_BUILD_AFTER_450:
                    enable_tutorial2_after_450(menu_item, type);
                    break;
                default:
                    enable_normal(menu_item, type);
                    break;
            }
            disable_resources(menu_item, type);

            if (*menu_item) {
                int submenu = building_menu_get_submenu_for_type(type);
                if (submenu) {
                    disable_if_no_enabled_submenu_items(menu_item, submenu);
                }
            }
        }
    }
    changed = 1;
}

int building_menu_count_items(int submenu)
{
    int count = 0;
    for (int item = 0; item < BUILD_MENU_ITEM_MAX; item++) {
        if (menu_enabled[submenu][item] && MENU_BUILDING_TYPE[submenu][item] > 0) {
            count++;
        }
    }
    return count;
}

int building_menu_count_all_items(int submenu)
{
    int count = 0;
    for (int item = 0; item < BUILD_MENU_ITEM_MAX; item++) {
        if (MENU_BUILDING_TYPE[submenu][item] > 0) {
            count++;
        }
    }
    return count;
}

int building_menu_next_index(int submenu, int current_index)
{
    for (int i = current_index + 1; i < BUILD_MENU_ITEM_MAX; i++) {
        if (MENU_BUILDING_TYPE[submenu][i] <= 0) {
            return 0;
        }
        if (menu_enabled[submenu][i]) {
            return i;
        }
    }
    return 0;
}

building_type building_menu_type(int submenu, int item)
{
    return MENU_BUILDING_TYPE[submenu][item];
}

build_menu_group building_menu_for_type(building_type type)
{
    for (int sub = 0; sub < BUILD_MENU_MAX; sub++) {
        for (int item = 0; item < BUILD_MENU_ITEM_MAX && MENU_BUILDING_TYPE[sub][item]; item++) {
            if (MENU_BUILDING_TYPE[sub][item] == type) {
                return sub;
            }
        }
    }
    return SUBMENU_NONE;
}

int building_menu_is_enabled(building_type type)
{
    for (int sub = 0; sub < BUILD_MENU_MAX; sub++) {
        for (int item = 0; item < BUILD_MENU_ITEM_MAX && MENU_BUILDING_TYPE[sub][item]; item++) {
            if (MENU_BUILDING_TYPE[sub][item] != type) {
                continue;
            }
            if (!menu_enabled[sub][item]) {
                return 0;
            }
            if (BUILD_MENU_TYPE_TO_BUILDING_TYPE[sub] != BUILDING_NONE) {
                return building_menu_is_enabled(BUILD_MENU_TYPE_TO_BUILDING_TYPE[sub]);
            } else {
                return 1;
            }
        }
    }
    return 0;
}

int building_menu_has_changed(void)
{
    if (changed) {
        changed = 0;
        return 1;
    }
    return 0;
}

int building_menu_is_submenu(build_menu_group menu)
{
    return BUILD_MENU_TYPE_TO_BUILDING_TYPE[menu] != BUILDING_NONE;
}

int building_menu_get_submenu_for_type(building_type type)
{
    switch (type) {
        case BUILDING_MENU_FARMS:
            return BUILD_MENU_FARMS;
        case BUILDING_MENU_RAW_MATERIALS:
            return BUILD_MENU_RAW_MATERIALS;
        case BUILDING_MENU_WORKSHOPS:
            return BUILD_MENU_WORKSHOPS;
        case BUILDING_MENU_SMALL_TEMPLES:
            return BUILD_MENU_SMALL_TEMPLES;
        case BUILDING_MENU_LARGE_TEMPLES:
            return BUILD_MENU_LARGE_TEMPLES;
        case BUILDING_FORT:
            return BUILD_MENU_FORTS;
        case BUILDING_MENU_GRAND_TEMPLES:
            return BUILD_MENU_GRAND_TEMPLES;
        case BUILDING_MENU_PARKS:
            return BUILD_MENU_PARKS;
        case BUILDING_MENU_TREES:
            return BUILD_MENU_TREES;
        case BUILDING_MENU_PATHS:
            return BUILD_MENU_PATHS;
        case BUILDING_MENU_GOV_RES:
            return BUILD_MENU_GOV_RES;
        case BUILDING_MENU_STATUES:
            return BUILD_MENU_STATUES;
        case BUILDING_MENU_SHRINES:
            return BUILD_MENU_SHRINES;
        case BUILDING_MENU_GARDENS:
            return BUILD_MENU_GARDENS;
        default:
            return 0;
    }
}
