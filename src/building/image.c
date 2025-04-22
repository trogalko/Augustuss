#include "image.h"

#include "assets/assets.h"
#include "building/connectable.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/variant.h"
#include "city/festival.h"
#include "city/view.h"
#include "core/direction.h"
#include "core/image.h"
#include "core/image_group.h"
#include "core/random.h"
#include "game/resource.h"
#include "map/random.h"
#include "scenario/property.h"

static const struct {
    int group;
    int offset;
    int num_types;
} HOUSE_IMAGE[20] = {
    {GROUP_BUILDING_HOUSE_TENT, 0, 2}, {GROUP_BUILDING_HOUSE_TENT, 2, 2},
    {GROUP_BUILDING_HOUSE_SHACK, 0, 2}, {GROUP_BUILDING_HOUSE_SHACK, 2, 2},
    {GROUP_BUILDING_HOUSE_HOVEL, 0, 2}, {GROUP_BUILDING_HOUSE_HOVEL, 2, 2},
    {GROUP_BUILDING_HOUSE_CASA, 0, 2}, {GROUP_BUILDING_HOUSE_CASA, 2, 2},
    {GROUP_BUILDING_HOUSE_INSULA_1, 0, 2}, {GROUP_BUILDING_HOUSE_INSULA_1, 2, 2},
    {GROUP_BUILDING_HOUSE_INSULA_2, 0, 2}, {GROUP_BUILDING_HOUSE_INSULA_2, 2, 2},
    {GROUP_BUILDING_HOUSE_VILLA_1, 0, 2}, {GROUP_BUILDING_HOUSE_VILLA_1, 2, 2},
    {GROUP_BUILDING_HOUSE_VILLA_2, 0, 1}, {GROUP_BUILDING_HOUSE_VILLA_2, 1, 1},
    {GROUP_BUILDING_HOUSE_PALACE_1, 0, 1}, {GROUP_BUILDING_HOUSE_PALACE_1, 1, 1},
    {GROUP_BUILDING_HOUSE_PALACE_2, 0, 1}, {GROUP_BUILDING_HOUSE_PALACE_2, 1, 1},
};

int building_image_get_base_farm_crop(building_type type)
{
    switch (type) {
        case BUILDING_WHEAT_FARM:
        case BUILDING_NATIVE_CROPS:
            return image_group(GROUP_BUILDING_FARM_CROPS);
        case BUILDING_VEGETABLE_FARM:
            return image_group(GROUP_BUILDING_FARM_CROPS) + 5;
        case BUILDING_FRUIT_FARM:
            return image_group(GROUP_BUILDING_FARM_CROPS) + 10;
        case BUILDING_OLIVE_FARM:
            return image_group(GROUP_BUILDING_FARM_CROPS) + 15;
        case BUILDING_VINES_FARM:
            return image_group(GROUP_BUILDING_FARM_CROPS) + 20;
        case BUILDING_PIG_FARM:
            return image_group(GROUP_BUILDING_FARM_CROPS) + 25;
        default:
            return 0;
    }
}

int building_image_get(const building *b)
{
    switch (b->type) {
        case BUILDING_HOUSE_VACANT_LOT:
            if (b->house_population == 0) {
                return image_group(GROUP_BUILDING_HOUSE_VACANT_LOT);
            }
            // fallthrough
        case BUILDING_HOUSE_LARGE_TENT:
        case BUILDING_HOUSE_SMALL_SHACK:
        case BUILDING_HOUSE_LARGE_SHACK:
        case BUILDING_HOUSE_SMALL_HOVEL:
        case BUILDING_HOUSE_LARGE_HOVEL:
        case BUILDING_HOUSE_SMALL_CASA:
        case BUILDING_HOUSE_LARGE_CASA:
        case BUILDING_HOUSE_SMALL_INSULA:
        case BUILDING_HOUSE_MEDIUM_INSULA:
        case BUILDING_HOUSE_LARGE_INSULA:
        case BUILDING_HOUSE_GRAND_INSULA:
        case BUILDING_HOUSE_SMALL_VILLA:
        case BUILDING_HOUSE_MEDIUM_VILLA:
        case BUILDING_HOUSE_LARGE_VILLA:
        case BUILDING_HOUSE_GRAND_VILLA:
        case BUILDING_HOUSE_SMALL_PALACE:
        case BUILDING_HOUSE_MEDIUM_PALACE:
        case BUILDING_HOUSE_LARGE_PALACE:
        case BUILDING_HOUSE_LUXURY_PALACE:
            {
                int image_id = image_group(HOUSE_IMAGE[b->subtype.house_level].group);
                if (b->house_is_merged) {
                    image_id += 4;
                    if (HOUSE_IMAGE[b->subtype.house_level].offset) {
                        image_id += 1;
                    }
                } else {
                    image_id += HOUSE_IMAGE[b->subtype.house_level].offset;
                    image_id += map_random_get(b->grid_offset) & (HOUSE_IMAGE[b->subtype.house_level].num_types - 1);
                }
                return image_id;
            }
        case BUILDING_AMPHITHEATER:
            if (!b->upgrade_level) {
                return assets_get_image_id("Health_Culture", "Amphitheatre ON");
            } else {
                return assets_get_image_id("Health_Culture", "Amphitheatre Upgrade ON");
            }
        case BUILDING_THEATER:
            if (!b->upgrade_level) {
                return assets_get_image_id("Health_Culture", "Theatre ON");
            } else {
                return assets_get_image_id("Health_Culture", "Theatre Upgrade ON");
            }
        case BUILDING_COLOSSEUM:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Colosseum_Construction_01");
                case 2:
                    return assets_get_image_id("Monuments", "Colosseum_Construction_02");
                case 3:
                    return assets_get_image_id("Monuments", "Colosseum_Construction_03");
                case 4:
                    return assets_get_image_id("Monuments", "Colosseum_Construction_04");
                default:
                    switch (city_festival_games_active()) {
                        case 1:
                            return assets_get_image_id("Monuments", "Col Naumachia");
                            break;
                        case 2:
                            return assets_get_image_id("Monuments", "Col Imp Games");
                            break;
                        case 3:
                            return assets_get_image_id("Monuments", "Col Exec");
                            break;
                        default:
                            return assets_get_image_id("Monuments", "Col Glad Fight");
                    }
            }
        case BUILDING_GLADIATOR_SCHOOL:
            return image_group(GROUP_BUILDING_GLADIATOR_SCHOOL);
        case BUILDING_LION_HOUSE:
            return image_group(GROUP_BUILDING_LION_HOUSE);
        case BUILDING_ACTOR_COLONY:
            return image_group(GROUP_BUILDING_ACTOR_COLONY);
        case BUILDING_CHARIOT_MAKER:
            return image_group(GROUP_BUILDING_CHARIOT_MAKER);
        case BUILDING_SMALL_STATUE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Aesthetics", "V Small Statue") +
                (orientation % 2) * building_properties_for_type(b->type)->rotation_offset;
        }
        case BUILDING_MEDIUM_STATUE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            switch (orientation % 2) {
                case 1:
                    return assets_get_image_id("Aesthetics", "Med_Statue_R");
                default:
                    return image_group(GROUP_BUILDING_STATUE) + 1;
            }
        }
        case BUILDING_LARGE_STATUE:
            return assets_get_image_id("Aesthetics", "l statue anim");
        case BUILDING_SMALL_POND:
            {
                int offset = b->has_water_access;
                if (scenario_property_climate() == CLIMATE_DESERT) {
                    return assets_get_image_id("Aesthetics", "s pond south off") + offset;
                } else {
                    return assets_get_image_id("Aesthetics", "s pond north off") + offset;
                }
            }
        case BUILDING_LARGE_POND:
            {
                int offset = b->has_water_access;
                if (scenario_property_climate() == CLIMATE_DESERT) {
                    return assets_get_image_id("Aesthetics", "l pond south off") + offset;
                } else {
                    return assets_get_image_id("Aesthetics", "l pond north off") + offset;
                }
            }
        case BUILDING_PAVILION_BLUE:
            return building_variant_get_image_id_with_rotation(b->type, b->variant);
        case BUILDING_PAVILION_RED:
            return assets_get_image_id("Aesthetics", "pavilion red");
        case BUILDING_PAVILION_ORANGE:
            return assets_get_image_id("Aesthetics", "pavilion orange");
        case BUILDING_PAVILION_YELLOW:
            return assets_get_image_id("Aesthetics", "pavilion yellow");
        case BUILDING_PAVILION_GREEN:
            return assets_get_image_id("Aesthetics", "pavilion green");
        case BUILDING_OBELISK:
            return assets_get_image_id("Aesthetics", "obelisk");
        case BUILDING_DOCTOR:
            return image_group(GROUP_BUILDING_DOCTOR);
        case BUILDING_HOSPITAL:
            return image_group(GROUP_BUILDING_HOSPITAL);
        case BUILDING_BATHHOUSE:
            if (b->has_water_access && b->num_workers) {
                if (!b->upgrade_level) {
                    return image_group(GROUP_BUILDING_BATHHOUSE_WATER);
                } else {
                    return image_group(GROUP_BUILDING_BATHHOUSE_FANCY_WATER);
                }
            } else {
                if (!b->upgrade_level) {
                    return image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER);
                } else {
                    return image_group(GROUP_BUILDING_BATHHOUSE_FANCY_NO_WATER);
                }
            }
        case BUILDING_BARBER:
            return image_group(GROUP_BUILDING_BARBER);
        case BUILDING_SCHOOL:
            if (!b->upgrade_level) {
                return image_group(GROUP_BUILDING_SCHOOL);
            } else {
                return assets_get_image_id("Health_Culture", "Upgraded_School");
            }
        case BUILDING_ACADEMY:
            if (!b->upgrade_level) {
                return assets_get_image_id("Health_Culture", "Academy_Fix");
            } else {
                return assets_get_image_id("Health_Culture", "Upgraded_Academy");
            }
        case BUILDING_LIBRARY:
            if (!b->upgrade_level) {
                return assets_get_image_id("Health_Culture", "Downgraded_Library");
            } else {
                return image_group(GROUP_BUILDING_LIBRARY);
            }
        case BUILDING_PREFECTURE:
            return image_group(GROUP_BUILDING_PREFECTURE);
        case BUILDING_MARBLE_QUARRY:
            return image_group(GROUP_BUILDING_MARBLE_QUARRY);
        case BUILDING_GOLD_MINE:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Industry", "Gold_Mine_N_ON");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Industry", "Gold_Mine_S_ON");
                default:
                    return assets_get_image_id("Industry", "Gold_Mine_C_ON");
            }
        case BUILDING_STONE_QUARRY:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Industry", "Stone_Quarry_N_ON");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Industry", "Stone_Quarry_S_ON");
                default:
                    return assets_get_image_id("Industry", "Stone_Quarry_C_ON");
            }
        case BUILDING_SAND_PIT:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Industry", "Sand_Pit_N_ON");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Industry", "Sand_Pit_S_ON");
                default:
                    return assets_get_image_id("Industry", "Sand_Pit_C_ON");
            }
        case BUILDING_BRICKWORKS:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Industry", "Brickworks_N_ON");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Industry", "Brickworks_S_ON");
                default:
                    return assets_get_image_id("Industry", "Brickworks_C_ON");
            }
        case BUILDING_CONCRETE_MAKER:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Industry", "Concrete_Maker_N_ON");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Industry", "Concrete_Maker_S_ON");
                default:
                    return assets_get_image_id("Industry", "Concrete_Maker_C_ON");
            }
        case BUILDING_CITY_MINT:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "City_Mint_Construction_01");
                case 2:
                    return assets_get_image_id("Monuments", "City_Mint_Construction_02");
                default:
                    return building_variant_get_image_id_with_rotation(b->type, b->variant);
            }
        case BUILDING_IRON_MINE:
            return image_group(GROUP_BUILDING_IRON_MINE);
        case BUILDING_TIMBER_YARD:
            return image_group(GROUP_BUILDING_TIMBER_YARD);
        case BUILDING_CLAY_PIT:
            return image_group(GROUP_BUILDING_CLAY_PIT);
        case BUILDING_WINE_WORKSHOP:
            return image_group(GROUP_BUILDING_WINE_WORKSHOP);
        case BUILDING_OIL_WORKSHOP:
            return image_group(GROUP_BUILDING_OIL_WORKSHOP);
        case BUILDING_WEAPONS_WORKSHOP:
            return image_group(GROUP_BUILDING_WEAPONS_WORKSHOP);
        case BUILDING_FURNITURE_WORKSHOP:
            return image_group(GROUP_BUILDING_FURNITURE_WORKSHOP);
        case BUILDING_POTTERY_WORKSHOP:
            return image_group(GROUP_BUILDING_POTTERY_WORKSHOP);
        case BUILDING_GRANARY:
            return image_group(GROUP_BUILDING_GRANARY);
        case BUILDING_MARKET:
            if (!b->upgrade_level) {
                return image_group(GROUP_BUILDING_MARKET);
            } else {
                return image_group(GROUP_BUILDING_MARKET_FANCY);
            }
        case BUILDING_GOVERNORS_HOUSE:
            return image_group(GROUP_BUILDING_GOVERNORS_HOUSE);
        case BUILDING_GOVERNORS_VILLA:
            return image_group(GROUP_BUILDING_GOVERNORS_VILLA);
        case BUILDING_GOVERNORS_PALACE:
            return image_group(GROUP_BUILDING_GOVERNORS_PALACE);
        case BUILDING_MISSION_POST:
            return image_group(GROUP_BUILDING_MISSION_POST);
        case BUILDING_ENGINEERS_POST:
            return image_group(GROUP_BUILDING_ENGINEERS_POST);
        case BUILDING_FORUM:
            if (!b->upgrade_level) {
                return image_group(GROUP_BUILDING_FORUM);
            } else {
                return assets_get_image_id("Admin_Logistics", "Upgraded_Forum");
            }
        case BUILDING_FOUNTAIN:
            if (b->upgrade_level == 3) {
                return scenario_property_climate() == CLIMATE_DESERT ?
                    assets_get_image_id("Admin_Logistics", "Fountain_Desert_Fix") :
                    image_group(GROUP_BUILDING_FOUNTAIN_4);
            } else if (b->upgrade_level == 2) {
                return image_group(GROUP_BUILDING_FOUNTAIN_3);
            } else if (b->upgrade_level == 1) {
                return image_group(GROUP_BUILDING_FOUNTAIN_2);
            } else {
                return image_group(GROUP_BUILDING_FOUNTAIN_1);
            }
        case BUILDING_WELL:
            return image_group(GROUP_BUILDING_WELL);
        case BUILDING_RESERVOIR:
            return image_group(GROUP_BUILDING_RESERVOIR);
        case BUILDING_MILITARY_ACADEMY:
            return image_group(GROUP_BUILDING_MILITARY_ACADEMY);
        case BUILDING_SMALL_TEMPLE_CERES:
            return image_group(GROUP_BUILDING_TEMPLE_CERES);
        case BUILDING_SMALL_TEMPLE_NEPTUNE:
            return image_group(GROUP_BUILDING_TEMPLE_NEPTUNE);
        case BUILDING_SMALL_TEMPLE_MERCURY:
            return image_group(GROUP_BUILDING_TEMPLE_MERCURY);
        case BUILDING_SMALL_TEMPLE_MARS:
            return image_group(GROUP_BUILDING_TEMPLE_MARS);
        case BUILDING_SMALL_TEMPLE_VENUS:
            return image_group(GROUP_BUILDING_TEMPLE_VENUS);
        case BUILDING_LARGE_TEMPLE_CERES:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Ceres_LT_0");
                case 2:
                    return assets_get_image_id("Monuments", "Ceres_LT_50");
                default:
                    return image_group(GROUP_BUILDING_TEMPLE_CERES) + 1;
            }
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Neptune_LT_0");
                case 2:
                    return assets_get_image_id("Monuments", "Neptune_LT_50");
                default:
                    return image_group(GROUP_BUILDING_TEMPLE_NEPTUNE) + 1;
            }
        case BUILDING_LARGE_TEMPLE_MERCURY:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Mercury_LT_0");
                case 2:
                    return assets_get_image_id("Monuments", "Mercury_LT_50");
                default:
                    return image_group(GROUP_BUILDING_TEMPLE_MERCURY) + 1;
            }
        case BUILDING_LARGE_TEMPLE_MARS:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Mars_LT_0");
                case 2:
                    return assets_get_image_id("Monuments", "Mars_LT_50");
                default:
                    return image_group(GROUP_BUILDING_TEMPLE_MARS) + 1;
            }
        case BUILDING_LARGE_TEMPLE_VENUS:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Venus_LT_0");
                case 2:
                    return assets_get_image_id("Monuments", "Venus_LT_50");
                default:
                    return image_group(GROUP_BUILDING_TEMPLE_VENUS) + 1;
            }
        case BUILDING_ORACLE:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Oracle_Construction_01");
                case 2:
                    return assets_get_image_id("Monuments", "Oracle_Construction_02");
                default:
                    return image_group(GROUP_BUILDING_ORACLE);
            }
        case BUILDING_LARARIUM:
            return assets_get_image_id("Health_Culture", "Lararium 01");
        case BUILDING_ROADBLOCK:
        case BUILDING_DECORATIVE_COLUMN:
            return building_variant_get_image_id_with_rotation(b->type, b->variant);
        case BUILDING_SHIPYARD:
            return image_group(GROUP_BUILDING_SHIPYARD) +
                ((4 + b->data.industry.orientation - city_view_orientation() / 2) % 4);
        case BUILDING_WHARF:
            return image_group(GROUP_BUILDING_WHARF) +
                ((4 + b->data.industry.orientation - city_view_orientation() / 2) % 4);
        case BUILDING_DOCK:
            switch ((4 + b->data.dock.orientation - city_view_orientation() / 2) % 4) {
                case 0:
                    return image_group(GROUP_BUILDING_DOCK_1);
                case 1:
                    return image_group(GROUP_BUILDING_DOCK_2);
                case 2:
                    return image_group(GROUP_BUILDING_DOCK_3);
                default:
                    return image_group(GROUP_BUILDING_DOCK_4);
            }
        case BUILDING_TOWER:
            return image_group(GROUP_BUILDING_TOWER);
        case BUILDING_GATEHOUSE:
            {
                int map_orientation = city_view_orientation();
                int orientation_is_top_bottom = map_orientation == DIR_0_TOP || map_orientation == DIR_4_BOTTOM;
                if (b->subtype.orientation == 1) {
                    if (orientation_is_top_bottom) {
                        return image_group(GROUP_BUILDING_TOWER) + 1;
                    } else {
                        return image_group(GROUP_BUILDING_TOWER) + 2;
                    }
                } else {
                    if (orientation_is_top_bottom) {
                        return image_group(GROUP_BUILDING_TOWER) + 2;
                    } else {
                        return image_group(GROUP_BUILDING_TOWER) + 1;
                    }
                }
            }
        case BUILDING_TRIUMPHAL_ARCH:
            {
                int map_orientation = city_view_orientation();
                int orientation_is_top_bottom = map_orientation == DIR_0_TOP || map_orientation == DIR_4_BOTTOM;
                if (b->subtype.orientation == 1) {
                    if (orientation_is_top_bottom) {
                        return image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
                    } else {
                        return image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
                    }
                } else {
                    if (orientation_is_top_bottom) {
                        return image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
                    } else {
                        return image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
                    }
                }
            }
        case BUILDING_SENATE:
            if (!b->upgrade_level) {
                return image_group(GROUP_BUILDING_SENATE);
            } else {
                return image_group(GROUP_BUILDING_SENATE_FANCY);
            }
        case BUILDING_BARRACKS:
            return image_group(GROUP_BUILDING_BARRACKS);
        case BUILDING_WAREHOUSE:
            return image_group(GROUP_BUILDING_WAREHOUSE);
        case BUILDING_WAREHOUSE_SPACE:
        {
            resource_type resource = b->subtype.warehouse_resource_id;
            if (resource == RESOURCE_NONE || b->resources[resource] <= 0) {
                return image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
            } else {
                return resource_get_data(b->subtype.warehouse_resource_id)->image.storage + b->resources[resource] - 1;
            }
        }
        case BUILDING_HIPPODROME:
            {
                int phase = b->monument.phase;
                if (!phase) {
                    phase = MONUMENT_FINISHED;
                }
                int phase_offset = 6;
                int orientation = building_rotation_get_building_orientation(b->subtype.orientation);
                int image_id;

                if (phase == MONUMENT_FINISHED) {
                    if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
                        image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
                    } else {
                        image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
                    }
                } else {
                    if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
                        image_id = assets_get_image_id("Monuments", "Circus NWSE 01") +
                            ((phase - 1) * phase_offset);
                    } else {
                        image_id = assets_get_image_id("Monuments", "Circus NESW 01") +
                            ((phase - 1) * phase_offset);
                    }
                }
                int building_part;
                if (b->prev_part_building_id == 0) {
                    building_part = 0; // part 1, no previous building
                } else if (b->next_part_building_id == 0) {
                    building_part = 2; // part 3, no next building
                } else {
                    building_part = 1; // part 2
                }

                if (orientation == DIR_0_TOP) {
                    switch (building_part) {
                        case 0: image_id += 0; break; // part 1
                        case 1: image_id += 2; break; // part 2
                        case 2: image_id += 4; break; // part 3, same for switch cases below
                    }
                } else if (orientation == DIR_4_BOTTOM) {
                    switch (building_part) {
                        case 0: image_id += 4; break;
                        case 1: image_id += 2; break;
                        case 2: image_id += 0; break;
                    }
                } else if (orientation == DIR_6_LEFT) {
                    switch (building_part) {
                        case 0: image_id += 0; break;
                        case 1: image_id += 2; break;
                        case 2: image_id += 4; break;
                    }
                } else { // DIR_2_RIGHT
                    switch (building_part) {
                        case 0: image_id += 4; break;
                        case 1: image_id += 2; break;
                        case 2: image_id += 0; break;
                    }
                }
                return image_id;
            }
        case BUILDING_FORT:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_MOUNTED:
        case BUILDING_FORT_AUXILIA_INFANTRY:
        case BUILDING_FORT_ARCHERS:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Military", "Fort_Main_North");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Military", "Fort_Main_South");
                default:
                    return assets_get_image_id("Military", "Fort_Main_Central");
            }
        case BUILDING_FORT_GROUND:
            return image_group(GROUP_BUILDING_FORT) + 1;
        case BUILDING_NATIVE_HUT:
            return image_group(GROUP_BUILDING_NATIVE) + (random_byte() & 1);
        case BUILDING_NATIVE_MEETING:
            return image_group(GROUP_BUILDING_NATIVE) + 2;
        case BUILDING_NATIVE_CROPS:
            return image_group(GROUP_BUILDING_FARM_CROPS);
        case BUILDING_GRAND_TEMPLE_CERES:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Ceres Complex Const 01");
                case 2:
                    return assets_get_image_id("Monuments", "Ceres Complex Const 02");
                case 3:
                    return assets_get_image_id("Monuments", "Ceres Complex Const 03");
                case 4:
                    return assets_get_image_id("Monuments", "Ceres Complex Const 04");
                case 5:
                    return assets_get_image_id("Monuments", "Ceres Complex Const 05");
                default:
                    switch (b->monument.upgrades) {
                        case 1:
                            return assets_get_image_id("Monuments", "Ceres Complex Module");
                        case 2:
                            return assets_get_image_id("Monuments", "Ceres Complex Module2");
                        default:
                            return assets_get_image_id("Monuments", "Ceres Complex On");
                    }
            }
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Neptune Complex Const 01");
                case 2:
                    return assets_get_image_id("Monuments", "Neptune Complex Const 02");
                case 3:
                    return assets_get_image_id("Monuments", "Neptune Complex Const 03");
                case 4:
                    return assets_get_image_id("Monuments", "Neptune Complex Const 04");
                case 5:
                    return assets_get_image_id("Monuments", "Neptune Complex Const 05");
                default:
                    switch (b->monument.upgrades) {
                        case 1:
                            return assets_get_image_id("Monuments", "Neptune Complex Module");
                        case 2:
                            return assets_get_image_id("Monuments", "Neptune Complex Module2");
                        default:
                            return assets_get_image_id("Monuments", "Neptune Complex On");
                    }
            }
        case BUILDING_GRAND_TEMPLE_MERCURY:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Mercury Complex Const 01");
                case 2:
                    return assets_get_image_id("Monuments", "Mercury Complex Const 02");
                case 3:
                    return assets_get_image_id("Monuments", "Mercury Complex Const 03");
                case 4:
                    return assets_get_image_id("Monuments", "Mercury Complex Const 04");
                case 5:
                    return assets_get_image_id("Monuments", "Mercury Complex Const 05");
                default:
                    switch (b->monument.upgrades) {
                        case 1:
                            return assets_get_image_id("Monuments", "Mercury Complex Module");
                        case 2:
                            return assets_get_image_id("Monuments", "Mercury Complex Module2");
                        default:
                            return assets_get_image_id("Monuments", "Mercury Complex On");
                    }
            }
        case BUILDING_GRAND_TEMPLE_MARS:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Mars Complex Const 01");
                case 2:
                    return assets_get_image_id("Monuments", "Mars Complex Const 02");
                case 3:
                    return assets_get_image_id("Monuments", "Mars Complex Const 03");
                case 4:
                    return assets_get_image_id("Monuments", "Mars Complex Const 04");
                case 5:
                    return assets_get_image_id("Monuments", "Mars Complex Const 05");
                default:
                    switch (b->monument.upgrades) {
                        case 1:
                            return assets_get_image_id("Monuments", "Mars Complex Module");
                        case 2:
                            return assets_get_image_id("Monuments", "Mars Complex Module2");
                        default:
                            return assets_get_image_id("Monuments", "Mars Complex On");
                    }
            }
        case BUILDING_GRAND_TEMPLE_VENUS:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Venus Complex Const 01");
                case 2:
                    return assets_get_image_id("Monuments", "Venus Complex Const 02");
                case 3:
                    return assets_get_image_id("Monuments", "Venus Complex Const 03");
                case 4:
                    return assets_get_image_id("Monuments", "Venus Complex Const 04");
                case 5:
                    return assets_get_image_id("Monuments", "Venus Complex Const 05");
                default:
                    switch (b->monument.upgrades) {
                        case 1:
                            return assets_get_image_id("Monuments", "Venus Complex Module");
                        case 2:
                            return assets_get_image_id("Monuments", "Venus Complex Module2");
                        default:
                            return assets_get_image_id("Monuments", "Venus Complex On");
                    }
            }
        case BUILDING_PANTHEON:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Pantheon Const 01");
                case 2:
                    return assets_get_image_id("Monuments", "Pantheon Const 02");
                case 3:
                    return assets_get_image_id("Monuments", "Pantheon Const 03");
                case 4:
                    return assets_get_image_id("Monuments", "Pantheon Const 04");
                case 5:
                    return assets_get_image_id("Monuments", "Pantheon Const 05");
                default:
                    switch (b->monument.upgrades) {
                        case 1:
                            return assets_get_image_id("Monuments", "Pantheon Module");
                        case 2:
                            return assets_get_image_id("Monuments", "Pantheon Module2");
                        default:
                            return assets_get_image_id("Monuments", "Pantheon On");
                    }
            }
        case BUILDING_LIGHTHOUSE:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Lighthouse_Construction_01");
                case 2:
                    return assets_get_image_id("Monuments", "Lighthouse_Construction_02");
                case 3:
                    return assets_get_image_id("Monuments", "Lighthouse_Construction_03");
                case 4:
                    return assets_get_image_id("Monuments", "Lighthouse_Construction_04");
                default:
                    if (b->resources[RESOURCE_TIMBER] > 0 && b->num_workers > 0) {
                        return assets_get_image_id("Monuments", "Lighthouse ON");
                    } else {
                        return assets_get_image_id("Monuments", "Lighthouse OFF");
                    }
            }
        case BUILDING_WORKCAMP:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Admin_Logistics", "Workcamp North");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Admin_Logistics", "Workcamp South");
                default:
                    return assets_get_image_id("Admin_Logistics", "Workcamp Central");
            }
        case BUILDING_ARCHITECT_GUILD:
            return assets_get_image_id("Admin_Logistics", "Arch Guild ON");
        case BUILDING_MESS_HALL:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Military", "Mess ON North");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Military", "Mess ON South");
                default:
                    return assets_get_image_id("Military", "Mess ON Central");
            }
        case BUILDING_TAVERN:
            if (!b->upgrade_level) {
                return assets_get_image_id("Health_Culture", "Tavern ON");
            } else {
                return assets_get_image_id("Health_Culture", "Tavern Upgrade ON");
            }
        case BUILDING_GRAND_GARDEN:
            return assets_get_image_id("Engineer", "Eng Guild ON");
        case BUILDING_ARENA:
            if (!b->upgrade_level) {
                return assets_get_image_id("Health_Culture", "Arena ON");
            } else {
                return assets_get_image_id("Health_Culture", "Arena Upgrade ON");
            }
        case BUILDING_HORSE_STATUE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Aesthetics", "Eque Statue") + orientation % 2;
        }
        case BUILDING_DOLPHIN_FOUNTAIN:
            return assets_get_image_id("Engineer", "Eng Guild ON");
        case BUILDING_LEGION_STATUE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Aesthetics", "legio statue") +
                (orientation % 2) * building_properties_for_type(b->type)->rotation_offset;
        }
        case BUILDING_WATCHTOWER:
        {
            int image_id = 0;
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    image_id = assets_get_image_id("Military", "Watchtower N ON");
                    break;
                case CLIMATE_DESERT:
                    image_id = assets_get_image_id("Military", "Watchtower S ON");
                    break;
                default:
                    image_id = assets_get_image_id("Military", "Watchtower C ON");
                    break;
            }
            
            int offset = building_variant_get_offset_with_rotation(b->type, b->variant);
            return image_id + offset;
        }
        case BUILDING_SMALL_MAUSOLEUM: {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Mausoleum_Small_Construction_01");
                case 2:
                    return assets_get_image_id("Monuments", "Mausoleum_Small_Construction_02") + orientation % 2;
                default:
                    return assets_get_image_id("Monuments", "Mausoleum S") + orientation % 2;
                }
            }
        case BUILDING_LARGE_MAUSOLEUM: {
            int offset = building_variant_get_offset_with_rotation(b->type, b->variant);
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Mausoleum L Cons");
                case 2:
                    return assets_get_image_id("Monuments", "Mausoleum_Large_Construction_02") + offset;
                default:
                    return assets_get_image_id("Monuments", "Mausoleum L") + offset;
            }
        }
        case BUILDING_NYMPHAEUM:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Pantheon_Const_00");
                case 2:
                    return assets_get_image_id("Monuments", "Nymphaeum_Construction_02");
                default:
                    return assets_get_image_id("Monuments", "Nymphaeum ON");
            }
        case BUILDING_CARAVANSERAI:
            switch (b->monument.phase) {
                case MONUMENT_START:
                    return assets_get_image_id("Monuments", "Caravanserai_Construction_01");
                case 2:
                    return assets_get_image_id("Monuments", "Caravanserai_Construction_02");
                default:
                    switch (scenario_property_climate()) {
                        case CLIMATE_DESERT:
                            return assets_get_image_id("Monuments", "Caravanserai_S_ON");
                        case CLIMATE_NORTHERN:
                            return assets_get_image_id("Monuments", "Caravanserai_N_ON");
                        default:
                            return assets_get_image_id("Monuments", "Caravanserai_C_ON");
                    }
            }
        case BUILDING_PINE_TREE:
        case BUILDING_FIR_TREE:
        case BUILDING_OAK_TREE:
        case BUILDING_ELM_TREE:
        case BUILDING_FIG_TREE:
        case BUILDING_PLUM_TREE:
        case BUILDING_PALM_TREE:
        case BUILDING_DATE_TREE:
            return assets_get_group_id("Aesthetics") + (b->type - BUILDING_PINE_TREE);
        case BUILDING_GODDESS_STATUE:
        case BUILDING_SENATOR_STATUE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Aesthetics", "sml statue 2") +
                (b->type - BUILDING_GODDESS_STATUE) + (orientation % 2) *
                building_properties_for_type(b->type)->rotation_offset;
        }
        case BUILDING_HEDGE_DARK:
            return assets_get_image_id("Aesthetics", "D Hedge 01") +
                building_connectable_get_hedge_offset(b->grid_offset);
        case BUILDING_HEDGE_LIGHT:
            return assets_get_image_id("Aesthetics", "L Hedge 01") +
                building_connectable_get_hedge_offset(b->grid_offset);
        case BUILDING_COLONNADE:
            return assets_get_image_id("Aesthetics", "G Colonnade 01") +
                building_connectable_get_colonnade_offset(b->grid_offset);
        case BUILDING_LOOPED_GARDEN_WALL:
            return assets_get_image_id("Aesthetics", "C Garden Wall 01") +
                building_connectable_get_garden_wall_offset(b->grid_offset);
        case BUILDING_ROOFED_GARDEN_WALL:
            return assets_get_image_id("Aesthetics", "R Garden Wall 01") +
                building_connectable_get_garden_wall_offset(b->grid_offset);
        case BUILDING_PANELLED_GARDEN_WALL:
            return assets_get_image_id("Aesthetics", "Garden_Wall_C_01") +
                building_connectable_get_garden_wall_offset(b->grid_offset);
        case BUILDING_DATE_PATH:
        case BUILDING_ELM_PATH:
        case BUILDING_FIG_PATH:
        case BUILDING_FIR_PATH:
        case BUILDING_OAK_PATH:
        case BUILDING_PALM_PATH:
        case BUILDING_PINE_PATH:
        case BUILDING_PLUM_PATH:
        case BUILDING_GARDEN_PATH:
            {
                int image_offset = building_connectable_get_garden_path_offset(b->grid_offset,
                    CONTEXT_GARDEN_PATH_INTERSECTION);
                int image_group = assets_get_image_id("Aesthetics", "Garden Path 01");
                // If path isn't an intersection, it's a straight path instead
                if (image_offset == -1) {
                    image_offset = building_connectable_get_garden_path_offset(b->grid_offset,
                        b->type == BUILDING_GARDEN_PATH ? CONTEXT_GARDEN_TREELESS_PATH : CONTEXT_GARDEN_TREE_PATH);

                    switch (b->type) {
                        case BUILDING_DATE_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn date");
                            break;
                        case BUILDING_ELM_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn elm");
                            break;
                        case BUILDING_FIG_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn fig");
                            break;
                        case BUILDING_FIR_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn fir");
                            break;
                        case BUILDING_OAK_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn oak");
                            break;
                        case BUILDING_PALM_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn palm");
                            break;
                        case BUILDING_PINE_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn pine");
                            break;
                        case BUILDING_PLUM_PATH:
                            image_group = assets_get_image_id("Aesthetics", "path orn plum");
                            break;
                        default:
                            image_group = assets_get_image_id("Aesthetics", "garden path r");
                            break;
                    }
                }
                return image_group + image_offset;
            }
        case BUILDING_BURNING_RUIN:
            if (b->data.rubble.was_tent) {
                return image_group(GROUP_TERRAIN_RUBBLE_TENT);
            } else {
                return image_group(GROUP_TERRAIN_RUBBLE_GENERAL) + 9 * (map_random_get(b->grid_offset) & 3);
            }
        case BUILDING_ROOFED_GARDEN_WALL_GATE:
            return assets_get_image_id("Aesthetics", "Garden_Gate_B") + building_connectable_get_garden_gate_offset(b->grid_offset);
        case BUILDING_LOOPED_GARDEN_GATE:
            return assets_get_image_id("Aesthetics", "Garden_Gate_A") + building_connectable_get_garden_gate_offset(b->grid_offset);
        case BUILDING_PANELLED_GARDEN_GATE:
            return assets_get_image_id("Aesthetics", "Garden_Gate_C") + building_connectable_get_garden_gate_offset(b->grid_offset);
        case BUILDING_HEDGE_GATE_LIGHT:
            return assets_get_image_id("Aesthetics", "L Hedge Gate") + building_connectable_get_hedge_gate_offset(b->grid_offset);
        case BUILDING_HEDGE_GATE_DARK:
            return assets_get_image_id("Aesthetics", "D Hedge Gate") + building_connectable_get_hedge_gate_offset(b->grid_offset);
        case BUILDING_PALISADE:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Military", "Pal Wall N 01") + building_connectable_get_palisade_offset(b->grid_offset);
                case CLIMATE_DESERT:
                    return assets_get_image_id("Military", "Pal Wall S 01") + building_connectable_get_palisade_offset(b->grid_offset);
                default:
                    return assets_get_image_id("Military", "Pal Wall C 01") + building_connectable_get_palisade_offset(b->grid_offset);
            }
        case BUILDING_PALISADE_GATE:
            return assets_get_image_id("Military", "Palisade_Gate") + building_connectable_get_palisade_gate_offset(b->grid_offset);
        case BUILDING_GLADIATOR_STATUE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Aesthetics", "Gladiator_Statue") +
                (orientation % 2) * building_properties_for_type(b->type)->rotation_offset;
        }
        case BUILDING_HIGHWAY:
            return assets_get_image_id("Admin_Logistics", "Highway_Placement");
        case BUILDING_DEPOT:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Admin_Logistics", "Cart Depot N ON");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Admin_Logistics", "Cart Depot S ON");
                default:
                    return assets_get_image_id("Admin_Logistics", "Cart Depot C ON");
            }
        case BUILDING_SHRINE_CERES:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Health_Culture", "Altar_Ceres") + orientation % 2;
        }
        case BUILDING_SHRINE_MARS:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Health_Culture", "Altar_Mars") + orientation % 2;
        }
        case BUILDING_SHRINE_MERCURY:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Health_Culture", "Altar_Mercury") + orientation % 2;
        }
        case BUILDING_SHRINE_NEPTUNE:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Health_Culture", "Altar_Neptune") + orientation % 2;
        }
        case BUILDING_SHRINE_VENUS:
        {
            int orientation = building_rotation_get_building_orientation(b->subtype.orientation) / 2;
            return assets_get_image_id("Health_Culture", "Altar_Venus") + orientation % 2;
        }
        case BUILDING_OVERGROWN_GARDENS:
            return building_properties_for_type(BUILDING_OVERGROWN_GARDENS)->image_group;

        case BUILDING_ARMOURY:
            switch (scenario_property_climate()) {
                case CLIMATE_NORTHERN:
                    return assets_get_image_id("Military", "Armoury_ON_N");
                case CLIMATE_DESERT:
                    return assets_get_image_id("Military", "Armoury_ON_S");
                default:
                    return assets_get_image_id("Military", "Armoury_ON_C");
            }

        default:
            return 0;
    }
}
