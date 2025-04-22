#include "select_scenario_action_type.h"

#include "core/string.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/event/parameter_data.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"

#define MAX_BUTTONS 10
#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define DETAILS_Y_OFFSET 160
#define DETAILS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 10

static void init(scenario_action_t *action);
static void on_scroll(void);
static void populate_list(int offset);
static void button_click(const generic_button *button);

static scrollbar_type scrollbar = {
    640, DETAILS_Y_OFFSET, DETAILS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (0 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (1 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 1},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (2 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 2},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (3 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 3},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (4 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 4},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (5 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 5},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (6 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 6},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (7 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 7},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (8 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 8},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (9 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 9}
};

static struct {
    unsigned int focus_button_id;
    unsigned int total_types;
    scenario_action_t *action;
    scenario_action_data_t *list[MAX_VISIBLE_ROWS];
} data;

static void init(scenario_action_t *action)
{
    data.action = action;
    data.total_types = ACTION_TYPE_MAX;

    scrollbar_init(&scrollbar, 0, data.total_types - 1);
    populate_list(0);
}

static void populate_list(int offset)
{
    if (data.total_types - offset < MAX_VISIBLE_ROWS) {
        offset = data.total_types - MAX_VISIBLE_ROWS;
    }
    if (offset < 0) {
        offset = 0;
    }
    for (unsigned int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        unsigned int target_index = i + offset;
        if (target_index < data.total_types) {
            data.list[i] = scenario_events_parameter_data_get_actions_xml_attributes_alphabetical(target_index);
        }
    }
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 42, 33);

    int y_offset = DETAILS_Y_OFFSET;
    for (unsigned int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (data.list[i]) {
            large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
            if (data.focus_button_id == (i + 1)) {
                button_border_draw(BUTTON_LEFT_PADDING, y_offset, BUTTON_WIDTH, DETAILS_ROW_HEIGHT, 1);
            }

            translation_key key = data.list[i]->xml_attr.key;
            text_draw(translation_for(key), 48, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
        }

        y_offset += DETAILS_ROW_HEIGHT;
    }

    lang_text_draw_centered(13, 3, 48, 32 + 16 * 30, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static void on_scroll(void)
{
    window_request_refresh();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
    populate_list(scrollbar.scroll_position);
}

static void button_click(const generic_button *button)
{
    int param1 = button->parameter1;
    data.action->type = data.list[param1]->type;
    data.action->parameter1 = scenario_events_parameter_data_get_default_value_for_parameter(&data.list[param1]->xml_parm1);
    data.action->parameter2 = scenario_events_parameter_data_get_default_value_for_parameter(&data.list[param1]->xml_parm2);
    data.action->parameter3 = scenario_events_parameter_data_get_default_value_for_parameter(&data.list[param1]->xml_parm3);
    data.action->parameter4 = scenario_events_parameter_data_get_default_value_for_parameter(&data.list[param1]->xml_parm4);
    data.action->parameter5 = scenario_events_parameter_data_get_default_value_for_parameter(&data.list[param1]->xml_parm5);
    window_go_back();
}

void window_editor_select_scenario_action_type_show(scenario_action_t *action)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_ACTION_TYPE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(action);
    window_show(&window);
}
