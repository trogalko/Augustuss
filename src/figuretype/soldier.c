#include "soldier.h"

#include "assets/assets.h"
#include "city/figures.h"
#include "city/games.h"
#include "city/map.h"
#include "core/calc.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/formation.h"
#include "figure/formation_layout.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figuretype/missile.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/point.h"

static const map_point ALTERNATIVE_POINTS[] = { {-1, -6},
    {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1},
    {0, -2}, {1, -2}, {2, -2}, {2, -1}, {2, 0}, {2, 1}, {2, 2}, {1, 2},
    {0, 2}, {-1, 2}, {-2, 2}, {-2, 1}, {-2, 0}, {-2, -1}, {-2, -2}, {-1, -2},
    {0, -3}, {1, -3}, {2, -3}, {3, -3}, {3, -2}, {3, -1}, {3, 0}, {3, 1},
    {3, 2}, {3, 3}, {2, 3}, {1, 3}, {0, 3}, {-1, 3}, {-2, 3}, {-3, 3},
    {-3, 2}, {-3, 1}, {-3, 0}, {-3, -1}, {-3, -2}, {-3, -3}, {-2, -3}, {-1, -3},
    {0, -4}, {1, -4}, {2, -4}, {3, -4}, {4, -4}, {4, -3}, {4, -2}, {4, -1},
    {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {3, 4}, {2, 4}, {1, 4},
    {0, 4}, {-1, 4}, {-2, 4}, {-3, 4}, {-4, 4}, {-4, 3}, {-4, 2}, {-4, 1},
    {-4, 0}, {-4, -1}, {-4, -2}, {-4, -3}, {-4, -4}, {-3, -4}, {-2, -4}, {-1, -4},
    {0, -5}, {1, -5}, {2, -5}, {3, -5}, {4, -5}, {5, -5}, {5, -4}, {5, -3},
    {5, -2}, {5, -1}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5},
    {4, 5}, {3, 5}, {2, 5}, {1, 5}, {0, 5}, {-1, 5}, {-2, 5}, {-3, 5},
    {-4, 5}, {-5, 5}, {-5, 4}, {-5, 3}, {-5, 2}, {-5, 1}, {-5, 0}, {-5, -1},
    {-5, -2}, {-5, -3}, {-5, -4}, {-5, -5}, {-4, -5}, {-3, -5}, {-2, -5}, {-1, -5},
    {0, -6}, {1, -6}, {2, -6}, {3, -6}, {4, -6}, {5, -6}, {6, -6}, {6, -5},
    {6, -4}, {6, -3}, {6, -2}, {6, -1}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
    {6, 4}, {6, 5}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6},
    {0, 6}, {-1, 6}, {-2, 6}, {-3, 6}, {-4, 6}, {-5, 6}, {-6, 6}, {-6, 5},
    {-6, 4}, {-6, 3}, {-6, 2}, {-6, 1}, {-6, 0}, {-6, -1}, {-6, -2}, {-6, -3},
    {-6, -4}, {-6, -5}, {-6, -6}, {-5, -6}, {-4, -6}, {-3, -6}, {-2, -6}, {-1, -6},
};



void figure_military_standard_action(figure *f)
{
    const formation *m = formation_get(f->formation_id);

    f->terrain_usage = TERRAIN_USAGE_ANY;
    figure_image_increase_offset(f, 16);
    map_figure_delete(f);
    if (m->is_at_fort) {
        f->x = m->x;
        f->y = m->y;
    } else {
        f->x = m->standard_x;
        f->y = m->standard_y;
    }
    f->grid_offset = map_grid_offset(f->x, f->y);
    f->cross_country_x = 15 * f->x + 7;
    f->cross_country_y = 15 * f->y + 7;
    map_figure_add(f);

    int pole_offset = 20 - m->morale / 5;
    if (pole_offset < 0) {
        pole_offset = 0;
    }
    f->image_id = image_group(GROUP_FIGURE_FORT_STANDARD_POLE) + pole_offset;

    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        if (m->is_halted) {
            f->cart_image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + 8;
        } else {
            f->cart_image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + f->image_offset / 2;
        }
    } else if (m->figure_type == FIGURE_FORT_MOUNTED) {
        if (m->is_halted) {
            f->cart_image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + 26;
        } else {
            f->cart_image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + 18 + f->image_offset / 2;
        }
    } else if (m->figure_type == FIGURE_FORT_JAVELIN) {
        if (m->is_halted) {
            f->cart_image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + 17;
        } else {
            f->cart_image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + 9 + f->image_offset / 2;
        }
    } else if (m->figure_type == FIGURE_FORT_INFANTRY) {
        if (m->is_halted) {
            f->cart_image_id = assets_get_image_id("UI", "auxinf_banner_0");
        } else {
            f->cart_image_id = assets_get_image_id("UI", "auxinf_banner_01") + f->image_offset / 2;
        }
    } else {
        if (m->is_halted) {
            f->cart_image_id = assets_get_image_id("UI", "auxarch_banner_0");
        } else {
            f->cart_image_id = assets_get_image_id("UI", "auxarch_banner_01") + f->image_offset / 2;
        }
    }
}

static int ticks_to_shoot(figure *f)
{
    switch (f->type) {
        case FIGURE_FORT_LEGIONARY:
            return 19;
        default:
            return 1;
    }
}

static figure *soldier_launch_missile(figure *f)
{
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        return f;
    }
    int range = 10;
    int projectile_type = FIGURE_JAVELIN;
    if (f->type == FIGURE_FORT_ARCHER) {
        range = 12;
        projectile_type = FIGURE_FRIENDLY_ARROW;
    } else if (f->type == FIGURE_FORT_LEGIONARY) {
        range = 6;
    }
    int missile_delay = figure_properties_for_type(f->type)->missile_delay;


    map_point tile = { -1, -1 };
    f->wait_ticks_missile++;
    if (f->wait_ticks_missile > missile_delay) {
        f->wait_ticks_missile = 0;
        if (figure_combat_get_missile_target_for_soldier(f, range, &tile)) {
            f->attack_image_offset = 1;
            f->direction = calc_missile_shooter_direction(f->x, f->y, tile.x, tile.y);
        } else {
            f->attack_image_offset = 0;
        }
    }
    if (f->attack_image_offset) {
        if (f->attack_image_offset == ticks_to_shoot(f)) {
            if (ticks_to_shoot(f) > 1) {
                // Adjust the target in case of long delay
                if (figure_combat_get_missile_target_for_soldier(f, range, &tile)) {
                    f->direction = calc_missile_shooter_direction(f->x, f->y, tile.x, tile.y);
                }
            }
            if (tile.x == -1 || tile.y == -1) {
                map_point_get_last_result(&tile);
            }
            int soldier_id = f->id;
            figure_create_missile(soldier_id, f->x, f->y, tile.x, tile.y, projectile_type);
            f = figure_get(f->id);
            formation_record_missile_fired(formation_get(f->formation_id));
        }
        f->attack_image_offset++;
        if (f->attack_image_offset > 100) {
            f->attack_image_offset = 0;
        }
    }
    return f;
}

static void legionary_attack_adjacent_enemy(figure *f)
{
    for (int i = 0; i < 8 && f->action_state != FIGURE_ACTION_150_ATTACK; i++) {
        figure_combat_attack_figure_at(f, f->grid_offset + map_grid_direction_delta(i));
    }
}

static int find_mop_up_target(figure *f)
{
    int target_id = f->target_figure_id;
    if (figure_is_dead(figure_get(target_id))) {
        f->target_figure_id = 0;
        target_id = 0;
    }
    if (target_id <= 0) {
        target_id = figure_combat_get_target_for_soldier(f->x, f->y, 20);
        if (target_id) {
            figure *target = figure_get(target_id);
            f->destination_x = target->x;
            f->destination_y = target->y;
            f->target_figure_id = target_id;
            target->targeted_by_figure_id = f->id;
            f->target_figure_created_sequence = target->created_sequence;
        } else {
            f->action_state = FIGURE_ACTION_84_SOLDIER_AT_STANDARD;
            f->image_offset = 0;
        }
        figure_route_remove(f);
    }
    return target_id;
}

static void update_image_javelin(figure *f, int dir)
{
    int image_id = image_group(GROUP_BUILDING_FORT_JAVELIN);
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->attack_image_offset < 12) {
            f->image_id = image_id + 96 + dir;
        } else {
            f->image_id = image_id + 96 + dir + 8 * ((f->attack_image_offset - 12) / 2);
        }
    } else if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_id + 144 + figure_image_corpse_offset(f);
    } else if (f->action_state == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        f->image_id = image_id + 96 + dir +
            8 * figure_image_missile_launcher_offset(f);
    } else {
        f->image_id = image_id + dir + 8 * f->image_offset;
    }
}

static void update_image_mounted(figure *f, int dir)
{
    int image_id = image_group(GROUP_FIGURE_FORT_MOUNTED);
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->attack_image_offset < 12) {
            f->image_id = image_id + 96 + dir;
        } else {
            f->image_id = image_id + 96 + dir + 8 * ((f->attack_image_offset - 12) / 2);
        }
    } else if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_id + 144 + figure_image_corpse_offset(f);
    } else {
        f->image_id = image_id + dir + 8 * f->image_offset;
    }
}

static void update_image_legionary(figure *f, const formation *m, int dir)
{
    int image_id = image_group(GROUP_BUILDING_FORT_LEGIONARY);
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->attack_image_offset < 12) {
            f->image_id = image_id + 96 + dir;
        } else {
            f->image_id = image_id + 96 + dir + 8 * ((f->attack_image_offset - 12) / 2);
        }
    } else if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_id + 152 + figure_image_corpse_offset(f);
    } else if (f->action_state == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        int missile_offset = figure_image_missile_launcher_offset(f);
        if (m->is_halted && m->layout == FORMATION_COLUMN && m->missile_attack_timeout) {
            f->image_id = image_id + dir + 144;
        } else if (missile_offset >= 0 && dir < DIR_8_NONE) {
            f->image_id = assets_get_image_id("Warriors", "legionary_fr_ne_01") + dir * 5 + missile_offset;
        } else {
            f->image_id = image_id + dir;
        }
    } else {
        f->image_id = image_id + dir + 8 * f->image_offset;
    }
}

static void update_image_infantry(figure *f, int dir)
{
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->attack_image_offset < 14) {
            f->image_id = assets_get_image_id("Warriors", "auxinf_f_ne_01") + dir * 5;
        } else {
            f->image_id = assets_get_image_id("Warriors", "auxinf_f_ne_01") + dir * 5 + ((f->attack_image_offset - 14) / 2);
        }
    } else if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = assets_get_image_id("Warriors", "auxinf_death_01") + figure_image_corpse_offset(f);
    } else {
        f->image_id = assets_get_image_id("Warriors", "auxinf_ne_01") + dir * 12 + f->image_offset;
    }
}

static void update_image_archer(figure *f, int dir)
{

    
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->attack_image_offset < 14) {
            f->image_id = assets_get_image_id("Warriors", "auxarch_fm_ne_01") + dir * 5;
        } else {
            f->image_id = assets_get_image_id("Warriors", "auxarch_fm_ne_01") + dir * 5 + ((f->attack_image_offset - 14) / 2);
        }
    } else if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = assets_get_image_id("Warriors", "auxarch_death_01") + figure_image_corpse_offset(f);
    } else if (f->action_state == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        int missile_offset = calc_bound(figure_image_missile_launcher_offset(f) - 1, 0, 4);
        f->image_id = assets_get_image_id("Warriors", "auxarch_fr_ne_01") + dir * 5 + missile_offset;
    } else {
        f->image_id = assets_get_image_id("Warriors", "auxarch_ne_01") + dir * 12 + f->image_offset;
    }

}



static void update_image(figure *f, const formation *m)
{
    int dir;
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        dir = f->attack_direction;
    } else if (m->missile_fired) {
        dir = f->direction;
    } else if (f->action_state == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        dir = m->direction;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previous_tile_direction;
    }
    dir = figure_image_normalize_direction(dir);
    if (f->type == FIGURE_FORT_JAVELIN) {
        update_image_javelin(f, dir);
    } else if (f->type == FIGURE_FORT_MOUNTED) {
        update_image_mounted(f, dir);
    } else if (f->type == FIGURE_FORT_LEGIONARY) {
        update_image_legionary(f, m, dir);
    } else if (f->type == FIGURE_FORT_INFANTRY) {
        update_image_infantry(f, dir);
    } else if (f->type == FIGURE_FORT_ARCHER) {
        update_image_archer(f, dir);
    }
}

static int soldier_percentage_speed(figure_type type)
{
    if (city_games_naval_battle_active()) {
        switch (type) {
        case FIGURE_FORT_LEGIONARY:
            return 25;
        case FIGURE_FORT_JAVELIN:
            return 50;
        case FIGURE_FORT_MOUNTED:
            return 75;
        case FIGURE_FORT_INFANTRY:
            return 85;
        case FIGURE_FORT_ARCHER:
            return 60;
            break;
        default:
            break;
        }
    }

    if (type == FIGURE_FORT_INFANTRY) {
        return 85;
    } else if (type == FIGURE_FORT_ARCHER) {
        return 85;
    }
    return 0;
}

void figure_soldier_action(figure *f)
{
    formation *m = formation_get(f->formation_id);
    city_figures_add_soldier();
    f->terrain_usage = TERRAIN_USAGE_ANY;
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    if (m->in_use != 1) {
        f->action_state = FIGURE_ACTION_149_CORPSE;
    }
    int speed_factor;
    int speed_factor_percentage = soldier_percentage_speed(f->type);
    if (f->type == FIGURE_FORT_MOUNTED) {
        speed_factor = 3;
    } else if (f->type == FIGURE_FORT_JAVELIN) {
        speed_factor = 2;
    } else {
        speed_factor = 1;
    }
    int layout = m->layout;
    if (f->formation_at_rest || f->action_state == FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT) {
        layout = FORMATION_AT_REST;
    }
    f->formation_position_x.soldier = m->x + formation_layout_position_x(layout, f->index_in_formation);
    f->formation_position_y.soldier = m->y + formation_layout_position_y(layout, f->index_in_formation);

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_80_SOLDIER_AT_REST:
            map_figure_update(f);
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->image_offset = 0;
            f->attack_image_offset = 0;
            if (f->x != f->formation_position_x.soldier || f->y != f->formation_position_y.soldier) {
                f->action_state = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            }
            break;
        case FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT:
        case FIGURE_ACTION_148_FLEEING:
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->destination_x = f->formation_position_x.soldier;
            f->destination_y = f->formation_position_y.soldier;
            f->destination_grid_offset = map_grid_offset(f->destination_x, f->destination_y);
            figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_80_SOLDIER_AT_REST;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_82_SOLDIER_RETURNING_TO_BARRACKS:
            f->formation_at_rest = 1;
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
            figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD:
            f->attack_image_offset = 0;
            f->formation_at_rest = 0;
            f->destination_x = m->standard_x + formation_layout_position_x(m->layout, f->index_in_formation);
            f->destination_y = m->standard_y + formation_layout_position_y(m->layout, f->index_in_formation);
            if (f->alternative_location_index) {
                f->destination_x += ALTERNATIVE_POINTS[f->alternative_location_index].x;
                f->destination_y += ALTERNATIVE_POINTS[f->alternative_location_index].y;
            }
            f->destination_grid_offset = map_grid_offset(f->destination_x, f->destination_y);
            figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_84_SOLDIER_AT_STANDARD;
                f->image_offset = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->alternative_location_index++;
                if (f->alternative_location_index > 168) {
                    f->state = FIGURE_STATE_DEAD;
                }
                f->image_offset = 0;
            }
            break;
        case FIGURE_ACTION_84_SOLDIER_AT_STANDARD:
            f->formation_at_rest = 0;
            f->image_offset = 0;
            map_figure_update(f);
            f->destination_x = m->standard_x + formation_layout_position_x(m->layout, f->index_in_formation);
            f->destination_y = m->standard_y + formation_layout_position_y(m->layout, f->index_in_formation);
            if (f->alternative_location_index) {
                f->destination_x += ALTERNATIVE_POINTS[f->alternative_location_index].x;
                f->destination_y += ALTERNATIVE_POINTS[f->alternative_location_index].y;
            }
            if (f->x != f->destination_x || f->y != f->destination_y) {
                if (m->missile_fired <= 0 && m->recent_fight <= 0 && m->missile_attack_timeout <= 0) {
                    f->action_state = FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD;
                    f->alternative_location_index = 0;
                }
            }
            if (f->action_state != FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD) {
                if (f->type == FIGURE_FORT_JAVELIN) {
                    f = soldier_launch_missile(f);
                } else if (f->type == FIGURE_FORT_ARCHER) {
                    f = soldier_launch_missile(f);
                } else if (f->type == FIGURE_FORT_LEGIONARY) {
                    legionary_attack_adjacent_enemy(f);
                    if (m->layout == FORMATION_DOUBLE_LINE_1 || m->layout == FORMATION_DOUBLE_LINE_2) {
                        f = soldier_launch_missile(f);
                    }
                } else if (f->type == FIGURE_FORT_INFANTRY || f->type == FIGURE_FORT_MOUNTED) {
                    legionary_attack_adjacent_enemy(f);
                }
            }
            break;
        case FIGURE_ACTION_85_SOLDIER_GOING_TO_MILITARY_ACADEMY:
            m->has_military_training = 1;
            f->formation_at_rest = 1;
            figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_86_SOLDIER_MOPPING_UP:
            f->formation_at_rest = 0;
            if (find_mop_up_target(f)) {
                figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
                if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                    figure *target = figure_get(f->target_figure_id);
                    f->destination_x = target->x;
                    f->destination_y = target->y;
                    figure_route_remove(f);
                } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                    f->action_state = FIGURE_ACTION_84_SOLDIER_AT_STANDARD;
                    f->target_figure_id = 0;
                    f->image_offset = 0;
                }
            }
            break;
        case FIGURE_ACTION_87_SOLDIER_GOING_TO_DISTANT_BATTLE:
            {
                const map_tile *exit = city_map_exit_point();
                f->formation_at_rest = 0;
                f->destination_x = exit->x;
                f->destination_y = exit->y;
                figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
                if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                    f->action_state = FIGURE_ACTION_89_SOLDIER_AT_DISTANT_BATTLE;
                    figure_route_remove(f);
                } else if (f->direction == DIR_FIGURE_REROUTE) {
                    figure_route_remove(f);
                } else if (f->direction == DIR_FIGURE_LOST) {
                    f->state = FIGURE_STATE_DEAD;
                }
                break;
            }
        case FIGURE_ACTION_88_SOLDIER_RETURNING_FROM_DISTANT_BATTLE:
            f->is_ghost = 0;
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->destination_x = f->formation_position_x.soldier;
            f->destination_y = f->formation_position_y.soldier;
            f->destination_grid_offset = map_grid_offset(f->destination_x, f->destination_y);
            figure_movement_move_ticks_with_percentage(f, speed_factor, speed_factor_percentage);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_80_SOLDIER_AT_REST;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_89_SOLDIER_AT_DISTANT_BATTLE:
            f->is_ghost = 1;
            f->formation_at_rest = 1;
            break;
    }

    update_image(f, m);
}
