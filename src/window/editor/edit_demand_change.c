#include "edit_demand_change.h"

#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/demand_change.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"

#include <stdlib.h>
#include <string.h>

static void button_year(const generic_button *button);
static void button_resource(const generic_button *button);
static void button_route(const generic_button *button);
static void button_amount(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_cancel(const generic_button *button);
static void button_save(const generic_button *button);

#define NUM_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year},
    {190, 152, 120, 25, button_resource},
    {420, 152, 200, 25, button_route},
    {350, 192, 100, 25, button_amount},
    {16, 238, 250, 25, button_delete},
    {409, 238, 100, 25, button_cancel},
    {524, 238, 100, 25, button_save}
};

#define MAX_POSSIBLE_ERRORS 4

static struct {
    demand_change_t demand_change;
    unsigned int focus_button_id;
    int *route_ids;
    const uint8_t **route_names;
    unsigned int num_routes;
    int is_new_demand_change;
    const uint8_t *errors[MAX_POSSIBLE_ERRORS];
    resource_type available_resources[RESOURCE_MAX];
} data;

static void create_route_info(int route_id, const uint8_t *city_name)
{
    int index = route_id - 1;
    data.route_ids[index] = route_id;
    int length = string_length(city_name) + 10;
    uint8_t *dst = malloc(sizeof(uint8_t) * length);
    if (!dst) {
        return;
    }
    int offset = string_from_int(dst, route_id, 0);
    dst[offset++] = ' ';
    dst[offset++] = '-';
    dst[offset++] = ' ';
    string_copy(city_name, &dst[offset], length - offset);
    data.route_names[index] = dst;
}

static void init(int id)
{
    for (unsigned int i = 0; i < data.num_routes; i++) {
        free((uint8_t *) data.route_names[i]);
    }
    free(data.route_ids);
    free(data.route_names);
    data.num_routes = trade_route_count() - 1;
    if (!data.num_routes) {
        data.route_ids = 0;
        data.route_names = 0;
        return;
    }
    data.route_ids = malloc(sizeof(int) * data.num_routes);
    data.route_names = malloc(sizeof(uint8_t *) * data.num_routes);
    if (!data.route_ids || !data.route_names) {
        return;
    }
    memset(data.route_ids, 0, sizeof(int) * data.num_routes);
    memset(data.route_names, 0, sizeof(uint8_t *) * data.num_routes);
    const demand_change_t *demand_change = scenario_demand_change_get(id);
    data.is_new_demand_change = demand_change->resource == RESOURCE_NONE;
    data.demand_change = *demand_change;

    for (int i = 1; i < trade_route_count(); i++) {
        empire_city *city = empire_city_get(empire_city_get_for_trade_route(i));
        if (city && (city->type == EMPIRE_CITY_TRADE || city->type == EMPIRE_CITY_FUTURE_TRADE)) {
            const uint8_t *city_name = empire_city_get_name(city);
            create_route_info(i, city_name);
        } else {
            create_route_info(i, lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_UNKNOWN_ROUTE));
        }
    }
}

static const uint8_t *get_text_for_route_id(int route_id)
{
    if (!data.num_routes) {
        return lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_NO_ROUTES);
    }
    // No route selected yet
    if (route_id == 0) {
        return lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_SET_A_ROUTE);
    }
    for (unsigned int i = 0; i < data.num_routes; i++) {
        if (data.route_ids[i] == route_id) {
            return data.route_names[i];
        }
    }
    return lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_UNKNOWN_ROUTE);
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 100, 40, 11);
    lang_text_draw(44, 94, 20, 114, FONT_LARGE_BLACK);

    text_draw_number_centered_prefix(data.demand_change.year, '+', 30, 158, 60, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.demand_change.year, 100, 158, FONT_NORMAL_BLACK);

    text_draw_centered(resource_get_data(data.demand_change.resource)->text, 190, 158, 120, FONT_NORMAL_BLACK,
        COLOR_MASK_NONE);

    lang_text_draw(44, 97, 330, 158, FONT_NORMAL_BLACK);
    text_draw_centered(get_text_for_route_id(data.demand_change.route_id), 420, 158, 200, FONT_NORMAL_BLACK, 0);

    lang_text_draw(44, 100, 60, 198, FONT_NORMAL_BLACK);
    text_draw_number_centered(data.demand_change.amount, 350, 198, 100, FONT_NORMAL_BLACK);

    lang_text_draw_centered_colored(44, 101, 16, 244, 250, FONT_NORMAL_PLAIN,
        data.is_new_demand_change ? COLOR_FONT_LIGHT_GRAY : COLOR_RED);

    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUTTON_CANCEL, 409, 244, 100, FONT_NORMAL_BLACK);

    lang_text_draw_centered(18, 3, 524, 244, 100, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    for (size_t i = 0; i < NUM_BUTTONS; i++) {
        int focus = data.focus_button_id == i + 1;
        if (i == 4 && data.is_new_demand_change) {
            focus = 0;
        }
        button_border_draw(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, focus);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, NUM_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_cancel(0);
        return;
    }
    if (h->enter_pressed) {
        button_save(0);
    }
}

static void set_year(int value)
{
    data.demand_change.year = value;
}

static void button_year(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 3, 999, set_year);
}

static void set_resource(int value)
{
    data.demand_change.resource = data.available_resources[value];
}

static void button_resource(const generic_button *button)
{
    static const uint8_t *resource_texts[RESOURCE_MAX];
    static int total_resources = 0;
    if (!total_resources) {
        for (resource_type resource = RESOURCE_NONE; resource < RESOURCE_MAX; resource++) {
            if (!resource_is_storable(resource)) {
                continue;
            }
            resource_texts[total_resources] = resource_get_data(resource)->text;
            data.available_resources[total_resources] = resource;
            total_resources++;
        }
    }
    window_select_list_show_text(screen_dialog_offset_x(), screen_dialog_offset_y(), button,
        resource_texts, total_resources, set_resource);
}

static void set_route_id(int index)
{
    data.demand_change.route_id = data.route_ids[index];
}

static void button_route(const generic_button *button)
{
    window_select_list_show_text(screen_dialog_offset_x(), screen_dialog_offset_y(), button,
        data.route_names, data.num_routes, set_route_id);
}

static void set_change_amount(int value)
{
    data.demand_change.amount = value;
}

static void button_amount(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 3, 999, set_change_amount);
}

static void button_delete(const generic_button *button)
{
    if (data.is_new_demand_change) {
        return;
    }
    scenario_demand_change_delete(data.demand_change.id);
    scenario_editor_set_as_unsaved();
    window_go_back();
}

static void button_cancel(const generic_button *button)
{
    window_go_back();
}

static int is_valid_route(void)
{
    if (!data.num_routes || data.demand_change.route_id == 0) {
        return 0;
    }
    for (unsigned int i = 0; i < data.num_routes; i++) {
        if (data.route_ids[i] == data.demand_change.route_id) {
            return 1;
        }
    }
    return 0;
}

static unsigned int validate(void)
{
    unsigned int num_errors = 0;

    for (int i = 0; i < MAX_POSSIBLE_ERRORS; i++) {
        data.errors[i] = 0;
    }

    if (data.demand_change.resource == RESOURCE_NONE) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_NO_RESOURCE);
    }
    if (data.demand_change.amount <= 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_NO_AMOUNT);
    }
    if (data.demand_change.year == 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_DEMAND_CHANGE_NO_YEAR);
    }
    if (!is_valid_route()) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_DEMAND_CHANGE_INVALID_ROUTE_SET);
    }

    return num_errors;
}

static void button_save(const generic_button *button)
{
    unsigned int num_errors = validate();
    if(num_errors) {
        window_plain_message_dialog_show_text_list(TR_EDITOR_FORM_ERRORS_FOUND, TR_EDITOR_FORM_HAS_FOLLOWING_ERRORS,
            data.errors, num_errors);
        return;
    }
    scenario_demand_change_update(&data.demand_change);
    scenario_editor_set_as_unsaved();
    window_go_back();
}

void window_editor_edit_demand_change_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_DEMAND_CHANGE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
