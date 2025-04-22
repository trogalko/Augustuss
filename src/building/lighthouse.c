#include "lighthouse.h"

#include "assets/assets.h"
#include "building/distribution.h"
#include "building/image.h"
#include "building/monument.h"
#include "city/trade_policy.h"
#include "core/calc.h"
#include "map/building_tiles.h"
#include "map/terrain.h"

#define INFINITE 10000
#define MAX_TIMBER 500
#define TIMBER_CONSUMPTION 20

int building_lighthouse_enough_timber(building *lighthouse)
{
    return lighthouse->resources[RESOURCE_TIMBER] > TIMBER_CONSUMPTION;
}

int building_lighthouse_get_storage_destination(building *lighthouse)
{
    if (lighthouse->resources[RESOURCE_TIMBER] >= MAX_TIMBER) {
        return 0;
    }

    resource_storage_info info[RESOURCE_MAX] = { 0 };
    info[RESOURCE_TIMBER].needed = 1;
    if (!building_distribution_get_resource_storages_for_building(info, lighthouse, INFINITE)) {
        return 0;
    }

    return info[RESOURCE_TIMBER].building_id;
}

int building_lighthouse_is_fully_functional(void)
{
    if (!building_monument_working(BUILDING_LIGHTHOUSE)) {
        return 0;
    }

    return building_lighthouse_enough_timber(building_first_of_type(BUILDING_LIGHTHOUSE));
}

static void set_lighthouse_graphic(building *b)
{
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    map_building_tiles_add(b->id, b->x, b->y, b->size, building_image_get(b), TERRAIN_BUILDING);
}

void building_lighthouse_consume_timber(void)
{
    if (building_monument_working(BUILDING_LIGHTHOUSE)) {
        building *b = building_get(building_find(BUILDING_LIGHTHOUSE));
        if (b->resources[RESOURCE_TIMBER] > 0) {
            trade_policy policy = city_trade_policy_get(SEA_TRADE_POLICY);
            int consume = TIMBER_CONSUMPTION;

            if (policy == TRADE_POLICY_3) { // consume 20% more
                consume = calc_adjust_with_percentage(consume, 100 + POLICY_3_MALUS_PERCENT);
            }

            if (b->resources[RESOURCE_TIMBER] - consume < 0) {
                b->resources[RESOURCE_TIMBER] = 0;
            } else {
                b->resources[RESOURCE_TIMBER] -= consume;
            }
        }
        set_lighthouse_graphic(b);
    }
}
