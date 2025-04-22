#include "select_city_trade_route.h"

#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
#include "empire/trade_route.h"
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
#include "window/editor/map.h"
#include "window/numeric_input.h"

#include <string.h>

#define MAX_BUTTONS 14
#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define DETAILS_Y_OFFSET 32
#define DETAILS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 14

static void on_scroll(void);
static void button_click(const generic_button *button);

static const uint8_t UNKNOWN[4] = { '?', '?', '?', 0 };

static scrollbar_type scrollbar = {
    640, DETAILS_Y_OFFSET, DETAILS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (0 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (1 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 1},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (2 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 2},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (3 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 3},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (4 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 4},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (5 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 5},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (6 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 6},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (7 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 7},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (8 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 8},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (9 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 9},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (10 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 10},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (11 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 11},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (12 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 12},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (13 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, 0, 13}
};

typedef struct {
    int route_id;
    const uint8_t *name;
} list_item_entry_t;

static struct {
    unsigned int focus_button_id;
    unsigned int list_size;
    void (*callback)(int);

    list_item_entry_t list[MAX_VISIBLE_ROWS];
} data;

static void populate_list(int offset)
{
    if (data.list_size - offset < MAX_VISIBLE_ROWS) {
        offset = data.list_size - MAX_VISIBLE_ROWS;
    }
    if (offset < 0) {
        offset = 0;
    }
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        unsigned int target_id = i + offset + 1;
        if (target_id < data.list_size) {
            data.list[i].route_id = target_id;
            int city_id = empire_city_get_for_trade_route(target_id);
            if (city_id != -1) {
                empire_city *city = empire_city_get(city_id);
                data.list[i].name = empire_city_get_name(city);
            } else {
                data.list[i].name = UNKNOWN;
            }
        } else {
            data.list[i].route_id = 0;
            data.list[i].name = UNKNOWN;
        }
    }
}

static void init(void (*callback)(int))
{
    data.callback = callback;
    data.list_size = trade_route_count();

    memset(data.list, 0, sizeof(data.list));

    scrollbar_init(&scrollbar, 0, data.list_size);
    populate_list(0);
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
        if (i < data.list_size - 1) {
            large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
            if (data.focus_button_id == (i + 1)) {
                button_border_draw(BUTTON_LEFT_PADDING, y_offset, BUTTON_WIDTH, DETAILS_ROW_HEIGHT, 1);
            }

            text_draw(data.list[i].name, 48, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
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
    int index = button->parameter1;

    if (index >= (int) data.list_size) {
        return;
    }

    data.callback(data.list[index].route_id);
    window_go_back();
}

void window_editor_select_city_trade_route_show(void (*callback)(int))
{
    window_type window = {
        WINDOW_EDITOR_SELECT_CITY_TADE_ROUTE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(callback);
    window_show(&window);
}
