#include "city.h"

#include "building/clone.h"
#include "building/construction.h"
#include "building/data_transfer.h"
#include "building/menu.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/type.h"
#include "building/variant.h"
#include "city/message.h"
#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/config.h"
#include "core/string.h"
#include "figure/formation.h"
#include "figure/formation_legion.h"
#include "figure/roamer_preview.h"
#include "game/orientation.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/time.h"
#include "game/undo.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/bookmark.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/terrain.h"
#include "scenario/allowed_building.h"
#include "scenario/criteria.h"
#include "widget/city.h"
#include "widget/city_with_overlay.h"
#include "widget/top_menu.h"
#include "widget/sidebar/city.h"
#include "widget/sidebar/extra.h"
#include "widget/sidebar/military.h"
#include "window/advisors.h"
#include "window/building_info.h"
#include "window/empire.h"
#include "window/file_dialog.h"
#include "window/message_list.h"

static int mothball_warning_id;

static void draw_background(void)
{
    if (window_is(WINDOW_CITY)) {
        widget_city_setup_routing_preview();
    }
    widget_sidebar_city_draw_background();
    widget_top_menu_draw(1);
}

static void draw_background_military(void)
{
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        widget_sidebar_military_draw_background();
    } else {
        widget_sidebar_city_draw_background();
    }
    widget_top_menu_draw(1);
}

static int center_in_city(int element_width_pixels)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    int margin = (width - element_width_pixels) / 2;
    return x + margin;
}

static int find_index(const uint8_t *string, char search)
{
    int index = 0;
    while (*string) {
        if (*string == search) {
            return index;
        }
        string++;
        index++;
    }
    return -1;
}

static int is_same_mapping(const hotkey_mapping *current, const hotkey_mapping *new)
{
    if (!new) {
        return current->key == KEY_TYPE_NONE;
    }
    return current->key == new->key && current->modifiers == new->modifiers;
}

static const uint8_t *get_paused_text(void)
{
    const uint8_t *pause_string = lang_get_string(13, 2);
    static uint8_t proper_hotkey_pause_string[300];
    static hotkey_mapping pause_key_mapping;
    const hotkey_mapping *new_mapping = hotkey_for_action(HOTKEY_TOGGLE_PAUSE, 0);
    if (*pause_string == *proper_hotkey_pause_string && is_same_mapping(&pause_key_mapping, new_mapping)) {
        return proper_hotkey_pause_string;
    }
    int parenthesis_index = find_index(pause_string, '(');
    if (parenthesis_index == -1) {
        return pause_string;
    }
    int p_key_index = find_index(pause_string + parenthesis_index, 'P');
    if (p_key_index == -1) {
        return pause_string;
    }
    uint8_t *cursor = string_copy(pause_string, proper_hotkey_pause_string, parenthesis_index + 1);
    pause_string += parenthesis_index;
    if (new_mapping) {
        pause_key_mapping.key = new_mapping->key;
        pause_key_mapping.modifiers = new_mapping->modifiers;
    } else {
        pause_key_mapping.key = KEY_TYPE_NONE;
        pause_key_mapping.modifiers = KEY_MOD_NONE;
    }
    if (pause_key_mapping.key == KEY_TYPE_NONE) {
        return proper_hotkey_pause_string;
    }
    cursor = string_copy(pause_string, cursor, p_key_index + 1);
    pause_string += p_key_index + 1;
    const uint8_t *keyname = key_combination_display_name(pause_key_mapping.key, pause_key_mapping.modifiers);
    cursor = string_copy(keyname, cursor, 300 - (cursor - proper_hotkey_pause_string));
    string_copy(pause_string, cursor, 300 - (cursor - proper_hotkey_pause_string));
    return proper_hotkey_pause_string;
}

static void draw_paused_banner(void)
{
    if (game_state_is_paused()) {
        int x_offset = center_in_city(448);
        outer_panel_draw(x_offset, 40, 28, 3);
        text_draw_centered(get_paused_text(), x_offset, 58, 448, FONT_NORMAL_BLACK, 0);
    }
}

static void draw_time_left(void)
{
    if (scenario_criteria_time_limit_enabled() && !city_victory_has_won()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int total_months = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 2, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK, 0);
    } else if (scenario_criteria_survival_enabled() && !city_victory_has_won()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int total_months = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 3, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK, 0);
    }
}

static void draw_speedrun_info(void)
{
    if (config_get(CONFIG_UI_SHOW_SPEEDRUN_INFO)) {
        int s_height = screen_height();
        large_label_draw(0, s_height - 25, 10, 0);
        lang_text_draw_centered(153, setting_difficulty() + 1, 4, s_height - 18, 150, FONT_NORMAL_WHITE);
    }
}

static void draw_foreground(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    widget_sidebar_city_draw_foreground();
    draw_speedrun_info();
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY)) {
        draw_time_left();
        widget_city_draw_construction_buttons();
        if (!mouse_get()->is_touch || sidebar_extra_is_information_displayed(SIDEBAR_EXTRA_DISPLAY_GAME_SPEED)) {
            draw_paused_banner();
        }
    }
    widget_city_draw_construction_cost_and_size();
    if (window_is(WINDOW_CITY)) {
        city_message_process_queue();
    }
}

static void draw_foreground_military(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        widget_sidebar_military_draw_foreground();
    } else {
        widget_sidebar_city_draw_foreground();
    }
    draw_time_left();
    widget_city_draw_construction_buttons();
    if (!mouse_get()->is_touch || sidebar_extra_is_information_displayed(SIDEBAR_EXTRA_DISPLAY_GAME_SPEED)) {
        draw_paused_banner();
    }
}

static void exit_military_command(void)
{
    if (window_is(WINDOW_CITY_MILITARY)) {
        window_city_show();
    }
}

static void show_roamers_for_overlay(int overlay)
{
    figure_roamer_preview_reset_building_types();

    switch (overlay) {
        case OVERLAY_FIRE:
        case OVERLAY_CRIME:
            figure_roamer_preview_create_all_for_building_type(BUILDING_PREFECTURE);
            break;
        case OVERLAY_DAMAGE:
            figure_roamer_preview_create_all_for_building_type(BUILDING_ENGINEERS_POST);
            break;
        case OVERLAY_TAVERN:
            figure_roamer_preview_create_all_for_building_type(BUILDING_TAVERN);
            break;
        case OVERLAY_THEATER:
            figure_roamer_preview_create_all_for_building_type(BUILDING_THEATER);
            break;
        case OVERLAY_AMPHITHEATER:
            figure_roamer_preview_create_all_for_building_type(BUILDING_AMPHITHEATER);
            break;
        case OVERLAY_ARENA:
            figure_roamer_preview_create_all_for_building_type(BUILDING_ARENA);
            break;
        case OVERLAY_COLOSSEUM:
            figure_roamer_preview_create_all_for_building_type(BUILDING_COLOSSEUM);
            break;
        case OVERLAY_HIPPODROME:
            figure_roamer_preview_create_all_for_building_type(BUILDING_HIPPODROME);
            break;
        case OVERLAY_SCHOOL:
            figure_roamer_preview_create_all_for_building_type(BUILDING_SCHOOL);
            break;
        case OVERLAY_LIBRARY:
            figure_roamer_preview_create_all_for_building_type(BUILDING_LIBRARY);
            break;
        case OVERLAY_ACADEMY:
            figure_roamer_preview_create_all_for_building_type(BUILDING_ACADEMY);
            break;
        case OVERLAY_BARBER:
            figure_roamer_preview_create_all_for_building_type(BUILDING_BARBER);
            break;
        case OVERLAY_BATHHOUSE:
            figure_roamer_preview_create_all_for_building_type(BUILDING_BATHHOUSE);
            break;
        case OVERLAY_CLINIC:
            figure_roamer_preview_create_all_for_building_type(BUILDING_DOCTOR);
            break;
        case OVERLAY_HOSPITAL:
            figure_roamer_preview_create_all_for_building_type(BUILDING_HOSPITAL);
            break;
        case OVERLAY_TAX_INCOME:
            figure_roamer_preview_create_all_for_building_type(BUILDING_FORUM);
            figure_roamer_preview_create_all_for_building_type(BUILDING_SENATE);
            break;
        case OVERLAY_FOOD_STOCKS:
            figure_roamer_preview_create_all_for_building_type(BUILDING_MARKET);
            break;
        case OVERLAY_SICKNESS:
            figure_roamer_preview_create_all_for_building_type(BUILDING_DOCTOR);
            figure_roamer_preview_create_all_for_building_type(BUILDING_HOSPITAL);
            break;
        case OVERLAY_NONE:
        default:
            break;
    }
    widget_city_clear_routing_grid_offset();
}

static void show_overlay(int overlay)
{
    exit_military_command();
    if (game_state_overlay() == overlay) {
        overlay = OVERLAY_NONE;
    }
    game_state_set_overlay(overlay);
    city_with_overlay_update();
    show_roamers_for_overlay(overlay);
    window_invalidate();
}

// this is mix of get_clone_type_from_grid_offset & get_clone_type_from_building functions with reduced code for overlay purpose
static int get_building_type_from_grid_offset(int grid_offset)
{
    int terrain = map_terrain_get(grid_offset);

    if (terrain & TERRAIN_BUILDING) {
        int building_id = map_building_at(grid_offset);
        if (building_id) {
            return building_main(building_get(building_id))->type;
        }
    } else if (terrain & TERRAIN_AQUEDUCT) {
        return BUILDING_AQUEDUCT;
    } else if (terrain & TERRAIN_GARDEN) {
        if (map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
            return BUILDING_OVERGROWN_GARDENS;
        }
        return BUILDING_GARDENS;
    } else if (terrain & TERRAIN_ROAD) {
        if (map_property_is_plaza_earthquake_or_overgrown_garden(grid_offset)) {
            return BUILDING_PLAZA;
        }
        return BUILDING_ROAD;
    } else if (terrain & TERRAIN_HIGHWAY) {
        return BUILDING_HIGHWAY;
    }

    return BUILDING_NONE;
}

static void show_overlay_from_grid_offset(int grid_offset)
{
    int overlay = OVERLAY_NONE;
    int clone_type = get_building_type_from_grid_offset(grid_offset);
    switch (clone_type) {
        case BUILDING_PLAZA:
        case BUILDING_ROAD:
        case BUILDING_ROADBLOCK:
        case BUILDING_ROOFED_GARDEN_WALL_GATE:
        case BUILDING_LOOPED_GARDEN_GATE:
        case BUILDING_PANELLED_GARDEN_GATE:
        case BUILDING_HIGHWAY:
            overlay = OVERLAY_ROADS;
            break;
        case BUILDING_AQUEDUCT:
        case BUILDING_RESERVOIR:
        case BUILDING_FOUNTAIN:
        case BUILDING_WELL:
            overlay = OVERLAY_WATER;
            break;
        case BUILDING_ORACLE:
        case BUILDING_SMALL_TEMPLE_CERES:
        case BUILDING_SMALL_TEMPLE_NEPTUNE:
        case BUILDING_SMALL_TEMPLE_MERCURY:
        case BUILDING_SMALL_TEMPLE_MARS:
        case BUILDING_SMALL_TEMPLE_VENUS:
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
        case BUILDING_PANTHEON:
        case BUILDING_LARARIUM:
        case BUILDING_NYMPHAEUM:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_LARGE_MAUSOLEUM:
        case BUILDING_SHRINE_CERES:
        case BUILDING_SHRINE_NEPTUNE:
        case BUILDING_SHRINE_MERCURY:
        case BUILDING_SHRINE_MARS:
        case BUILDING_SHRINE_VENUS:
            overlay = OVERLAY_RELIGION;
            break;
        case BUILDING_PREFECTURE:
        case BUILDING_BURNING_RUIN:
            overlay = OVERLAY_FIRE;
            break;
        case BUILDING_ENGINEERS_POST:
        case BUILDING_ARCHITECT_GUILD:
            overlay = OVERLAY_DAMAGE;
            break;
        case BUILDING_THEATER:
        case BUILDING_ACTOR_COLONY:
            overlay = OVERLAY_THEATER;
            break;
        case BUILDING_AMPHITHEATER:
        case BUILDING_GLADIATOR_SCHOOL:
            overlay = OVERLAY_AMPHITHEATER;
            break;
        case BUILDING_TAVERN:
            overlay = OVERLAY_TAVERN;
            break;
        case BUILDING_ARENA:
            overlay = OVERLAY_ARENA;
            break;
        case BUILDING_COLOSSEUM:
        case BUILDING_LION_HOUSE:
            overlay = OVERLAY_COLOSSEUM;
            break;
        case BUILDING_HIPPODROME:
        case BUILDING_CHARIOT_MAKER:
            overlay = OVERLAY_HIPPODROME;
            break;
        case BUILDING_SCHOOL:
            overlay = OVERLAY_SCHOOL;
            break;
        case BUILDING_LIBRARY:
            overlay = OVERLAY_LIBRARY;
            break;
        case BUILDING_ACADEMY:
            overlay = OVERLAY_ACADEMY;
            break;
        case BUILDING_BARBER:
            overlay = OVERLAY_BARBER;
            break;
        case BUILDING_BATHHOUSE:
            overlay = OVERLAY_BATHHOUSE;
            break;
        case BUILDING_DOCTOR:
            overlay = OVERLAY_CLINIC;
            break;
        case BUILDING_HOSPITAL:
            overlay = OVERLAY_HOSPITAL;
            break;
        case BUILDING_FORUM:
        case BUILDING_SENATE:
            overlay = OVERLAY_TAX_INCOME;
            break;
        case BUILDING_MARKET:
        case BUILDING_GRANARY:
        case BUILDING_CARAVANSERAI:
        case BUILDING_MESS_HALL:
        case BUILDING_FRUIT_FARM:
        case BUILDING_OLIVE_FARM:
        case BUILDING_PIG_FARM:
        case BUILDING_VEGETABLE_FARM:
        case BUILDING_VINES_FARM:
        case BUILDING_WHEAT_FARM:
        case BUILDING_OIL_WORKSHOP:
        case BUILDING_WINE_WORKSHOP:
        case BUILDING_WHARF:
            overlay = OVERLAY_FOOD_STOCKS;
            break;
        case BUILDING_GARDENS:
        case BUILDING_OVERGROWN_GARDENS:
        case BUILDING_GOVERNORS_HOUSE:
        case BUILDING_GOVERNORS_VILLA:
        case BUILDING_GOVERNORS_PALACE:
        case BUILDING_HOUSE_SMALL_TENT:
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
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_TRIUMPHAL_ARCH:
        case BUILDING_SMALL_POND:
        case BUILDING_LARGE_POND:
        case BUILDING_PINE_TREE:
        case BUILDING_FIR_TREE:
        case BUILDING_OAK_TREE:
        case BUILDING_ELM_TREE:
        case BUILDING_FIG_TREE:
        case BUILDING_PLUM_TREE:
        case BUILDING_PALM_TREE:
        case BUILDING_DATE_TREE:
        case BUILDING_PINE_PATH:
        case BUILDING_FIR_PATH:
        case BUILDING_OAK_PATH:
        case BUILDING_ELM_PATH:
        case BUILDING_FIG_PATH:
        case BUILDING_PLUM_PATH:
        case BUILDING_PALM_PATH:
        case BUILDING_DATE_PATH:
        case BUILDING_GARDEN_PATH:
        case BUILDING_PAVILION_BLUE:
        case BUILDING_PAVILION_RED:
        case BUILDING_PAVILION_ORANGE:
        case BUILDING_PAVILION_YELLOW:
        case BUILDING_PAVILION_GREEN:
        case BUILDING_GODDESS_STATUE:
        case BUILDING_SENATOR_STATUE:
        case BUILDING_OBELISK:
        case BUILDING_HORSE_STATUE:
        case BUILDING_LEGION_STATUE:
        case BUILDING_GLADIATOR_STATUE:
        case BUILDING_PANELLED_GARDEN_WALL:
            overlay = OVERLAY_DESIRABILITY;
            break;
        case BUILDING_MISSION_POST:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
            overlay = OVERLAY_NATIVE;
            break;
        case BUILDING_WAREHOUSE:
        case BUILDING_WAREHOUSE_SPACE:
        case BUILDING_DEPOT:
            overlay = OVERLAY_LOGISTICS;
            break;
        case BUILDING_DOCK:
            overlay = OVERLAY_SICKNESS;
            break;
        case BUILDING_NONE:
            if (map_terrain_get(grid_offset) & TERRAIN_RUBBLE) {
                overlay = OVERLAY_DAMAGE;
            }
            break;
        default:
            break;
    }
    if (!(game_state_overlay() == OVERLAY_NONE && overlay == OVERLAY_NONE)) {
        show_overlay(overlay);
    }
}

static int has_storage_orders(building_type type)
{
    return type == BUILDING_WAREHOUSE ||
        type == BUILDING_WAREHOUSE_SPACE ||
        type == BUILDING_GRANARY ||
        type == BUILDING_MARKET ||
        type == BUILDING_DOCK ||
        type == BUILDING_MESS_HALL ||
        type == BUILDING_TAVERN ||
        type == BUILDING_ROADBLOCK ||
        type == BUILDING_CARAVANSERAI ||
        type == BUILDING_ROOFED_GARDEN_WALL_GATE ||
        type == BUILDING_LOOPED_GARDEN_GATE ||
        type == BUILDING_PANELLED_GARDEN_GATE ||
        type == BUILDING_HEDGE_GATE_DARK ||
        type == BUILDING_HEDGE_GATE_LIGHT ||
        type == BUILDING_PALISADE_GATE ||
        (type == BUILDING_SMALL_TEMPLE_CERES && building_monument_gt_module_is_active(CERES_MODULE_2_DISTRIBUTE_FOOD)) ||
        (type == BUILDING_LARGE_TEMPLE_CERES && building_monument_gt_module_is_active(CERES_MODULE_2_DISTRIBUTE_FOOD)) ||
        (type == BUILDING_SMALL_TEMPLE_VENUS && building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) ||
        (type == BUILDING_LARGE_TEMPLE_VENUS && building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE));
}

static void cycle_legion(void)
{
    static int current_legion_id = 1;
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY)) {
        int legion_id = current_legion_id;
        current_legion_id = 0;
        for (int i = 1; i < formation_count(); i++) {
            legion_id++;
            if (legion_id > MAX_LEGIONS) {
                legion_id = 1;
            }
            const formation *m = formation_get(legion_id);
            if (m->in_use == 1 && !m->is_herd && m->is_legion) {
                if (current_legion_id == 0) {
                    current_legion_id = legion_id;
                    break;
                }
            }
        }
        if (current_legion_id > 0) {
            const formation *m = formation_get(current_legion_id);
            city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
            window_city_military_show(current_legion_id);
        }
    }
}

static void toggle_pause(void)
{
    game_state_toggle_paused();
    city_warning_clear_all();
}

static void set_construction_building_type(building_type type)
{
    if (scenario_allowed_building(type) && building_menu_is_enabled(type)) {
        building_construction_cancel();
        building_construction_set_type(type);
        window_request_refresh();
    }
}

static void handle_hotkeys(const hotkeys *h)
{
    if (h->toggle_pause) {
        toggle_pause();
    }
    if (h->decrease_game_speed) {
        setting_decrease_game_speed();
    }
    if (h->increase_game_speed) {
        setting_increase_game_speed();
    }
    if (h->show_overlay) {
        show_overlay(h->show_overlay);
    }
    if (h->show_overlay_relative) {
        show_overlay_from_grid_offset(widget_city_current_grid_offset());
    }
    if (h->toggle_overlay) {
        exit_military_command();
        game_state_toggle_overlay();
        show_roamers_for_overlay(game_state_overlay());
        city_with_overlay_update();
        window_invalidate();
    }
    if (h->show_advisor) {
        window_advisors_show_advisor(h->show_advisor);
    }
    if (h->cycle_legion) {
        cycle_legion();
    }
    if (h->rotate_map_left) {
        if (!building_construction_in_progress()) {
            game_orientation_rotate_left();
            window_invalidate();
        }
    }
    if (h->rotate_map_right) {
        if (!building_construction_in_progress()) {
            game_orientation_rotate_right();
            window_invalidate();
        }
    }
    if (h->rotate_map_north) {
        if (!building_construction_in_progress()) {
            game_orientation_rotate_north();
            window_invalidate();
        }
    }
    if (h->go_to_bookmark) {
        if (map_bookmark_go_to(h->go_to_bookmark - 1)) {
            window_invalidate();
        }
    }
    if (h->set_bookmark) {
        map_bookmark_save(h->set_bookmark - 1);
    }
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    }
    if (h->save_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
    }
    if (h->rotate_building) {
        building_rotation_rotate_forward();
    }
    if (h->rotate_building_back) {
        building_rotation_rotate_backward();
    }
    if (h->building) {
        set_construction_building_type(h->building);
    }
    if (h->undo) {
        game_undo_perform();
        window_invalidate();
    }
    if (h->mothball_toggle) {
        int building_id = map_building_at(widget_city_current_grid_offset());
        building *b = building_main(building_get(building_id));
        if (building_id && model_get_building(b->type)->laborers) {
            building_mothball_toggle(b);
            if (b->state == BUILDING_STATE_IN_USE) {
                mothball_warning_id = city_warning_show(WARNING_DATA_MOTHBALL_OFF, mothball_warning_id);
            } else if (b->state == BUILDING_STATE_MOTHBALLED) {
                mothball_warning_id = city_warning_show(WARNING_DATA_MOTHBALL_ON, mothball_warning_id);
            }
        }
    }
    if (h->storage_order) {
        int grid_offset = widget_city_current_grid_offset();
        int building_id = map_building_at(grid_offset);       
        if (building_id) {   
            building *b = building_main(building_get(building_id));
            if (has_storage_orders(b->type)) {
                    window_building_info_show(grid_offset);
                    window_building_info_show_storage_orders();
            }
        }
    }
    if (h->clone_building) {
        building_type type = building_clone_type_from_grid_offset(widget_city_current_grid_offset());
        if (type) {
            set_construction_building_type(type);
        }
    }
    if (h->copy_building_settings) {
        int building_id = map_building_at(widget_city_current_grid_offset());
        if (building_id) {
            building *b = building_main(building_get(building_id));
            building_data_transfer_copy(b);
        }
    }
    if (h->paste_building_settings) {
        int building_id = map_building_at(widget_city_current_grid_offset());
        if (building_id) {
            building *b = building_main(building_get(building_id));
            building_data_transfer_paste(b);
        }
    }
    if (h->show_empire_map) {
        if (!window_is(WINDOW_EMPIRE)) {
            window_empire_show_checked();
        }
    }
    if (h->show_messages) {
        window_message_list_show();
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    if (!building_construction_in_progress()) {
        if (widget_top_menu_handle_input(m, h)) {
            return;
        }
        if (widget_sidebar_city_handle_mouse(m)) {
            return;
        }
    }
    widget_city_handle_input(m, h);
}

static void handle_input_military(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    if (widget_top_menu_handle_input(m, h)) {
        return;
    }
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && widget_sidebar_military_handle_input(m)) {
        return;
    }
    widget_city_handle_input_military(m, h, formation_get_selected());
}

static void get_tooltip(tooltip_context *c)
{
    int text_id = widget_top_menu_get_tooltip_text(c);
    if (!text_id) {
        if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && formation_get_selected()) {
            text_id = widget_sidebar_military_get_tooltip_text(c);
        } else {
            text_id = widget_sidebar_city_get_tooltip_text(c);
        }
    }
    if (text_id || c->translation_key) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = text_id;
        return;
    }
    widget_city_get_tooltip(c);
}

int window_city_military_is_cursor_in_menu(void)
{
    if (!config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) || !window_is(WINDOW_CITY_MILITARY)) {
        return 0;
    }
    const mouse *m = mouse_get();
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    y += 24;
    height += 24;
    return m->x < x || m->x >= width || m->y < y || m->y >= height;
}

void window_city_draw_all(void)
{
    if (formation_get_selected() && config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        draw_background_military();
        draw_foreground_military();
    } else {
        draw_background();
        draw_foreground();
    }
}

void window_city_draw_panels(void)
{
    if (formation_get_selected() && config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        draw_background_military();
    } else {
        draw_background();
    }
}

void window_city_draw(void)
{
    widget_city_draw();
}

void window_city_show(void)
{
    show_roamers_for_overlay(game_state_overlay());
    if (formation_get_selected()) {
        formation_set_selected(0);
        if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && widget_sidebar_military_exit()) {
            return;
        }
    }
    window_type window = {
        WINDOW_CITY,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    window_show(&window);
}

void window_city_military_show(int legion_formation_id)
{
    if (building_construction_type()) {
        building_construction_cancel();
        building_construction_clear_type();
    }
    formation_set_selected(legion_formation_id);
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && widget_sidebar_military_enter(legion_formation_id)) {
        return;
    }
    window_type window = {
        WINDOW_CITY_MILITARY,
        draw_background_military,
        draw_foreground_military,
        handle_input_military,
        get_tooltip
    };
    window_show(&window);
}

void window_city_return(void)
{
    int formation_id = formation_get_selected();
    if (formation_id) {
        window_city_military_show(formation_id);
    } else {
        window_city_show();
    }
}
