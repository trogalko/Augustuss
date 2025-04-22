#include "set_salary.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "city/ratings.h"
#include "city/victory.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "window/advisors.h"

#define MIN_DIALOG_WIDTH 384

static void button_cancel(const generic_button *button);
static void button_set_salary(const generic_button *button);

static generic_button buttons[] = {
    {240, 395, 160, 20, button_cancel},
    {144, 85, 352, 20, button_set_salary, 0, 0},
    {144, 105, 352, 20, button_set_salary, 0, 1},
    {144, 125, 352, 20, button_set_salary, 0, 2},
    {144, 145, 352, 20, button_set_salary, 0, 3},
    {144, 165, 352, 20, button_set_salary, 0, 4},
    {144, 185, 352, 20, button_set_salary, 0, 5},
    {144, 205, 352, 20, button_set_salary, 0, 6},
    {144, 225, 352, 20, button_set_salary, 0, 7},
    {144, 245, 352, 20, button_set_salary, 0, 8},
    {144, 265, 352, 20, button_set_salary, 0, 9},
    {144, 285, 352, 20, button_set_salary, 0, 10},
};

static unsigned int focus_button_id;

static int get_dialog_width(void)
{
    int dialog_width = 16 + lang_text_get_width(52, 15, FONT_LARGE_BLACK);
    if (dialog_width < MIN_DIALOG_WIDTH) {
        dialog_width = MIN_DIALOG_WIDTH;
    }
    if (dialog_width % BLOCK_SIZE != 0) {
        // make sure the width is a multiple of 16
        dialog_width += BLOCK_SIZE - dialog_width % BLOCK_SIZE;
    }
    return dialog_width;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    int dialog_width = get_dialog_width();
    int dialog_x = 128 - (dialog_width - MIN_DIALOG_WIDTH) / 2;
    outer_panel_draw(dialog_x, 32, dialog_width / BLOCK_SIZE, 25);
    image_draw(resource_get_data(RESOURCE_DENARII)->image.icon, dialog_x + 10, 42, COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw_centered(52, 15, dialog_x + 25, 48, dialog_width - 64, FONT_LARGE_BLACK);

    inner_panel_draw(144, 80, 22, 15);

    for (unsigned int rank = 0; rank < 11; rank++) {
        font_t font = focus_button_id == rank + 2 ? FONT_NORMAL_RED : FONT_NORMAL_WHITE;
        int width = lang_text_draw(52, rank + 4, 176, 90 + 20 * rank, font);
        text_draw_money(city_emperor_salary_for_rank(rank), 176 + width, 90 + 20 * rank, font);
    }

    if (!city_victory_has_won()) {
        if (city_emperor_salary_rank() <= city_emperor_rank()) {
            lang_text_draw_multiline(52, 76, 152, 336, 336, FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_multiline(52, 71, 152, 336, 336, FONT_NORMAL_BLACK);
        }
    } else {
        lang_text_draw_multiline(52, 77, 150, 336, 340, FONT_NORMAL_BLACK);
    }
    button_border_draw(240, 395, 160, 20, focus_button_id == 1);
    lang_text_draw_centered(13, 4, 176, 400, 288, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 12, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_advisors_show();
    }
}

static void button_cancel(const generic_button *button)
{
    window_advisors_show();
}

static void button_set_salary(const generic_button *button)
{
    int rank = button->parameter1;

    if (!city_victory_has_won()) {
        city_emperor_set_salary_rank(rank);
        city_finance_update_salary();
        city_ratings_update_favor_explanation();
        window_advisors_show();
    }
}

void window_set_salary_show(void)
{
    window_type window = {
        WINDOW_SET_SALARY,
        window_advisors_draw_dialog_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
