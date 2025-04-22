#include "city_pause_menu.h"

#include "building/construction.h"
#include "core/lang.h"
#include "game/campaign.h"
#include "game/file.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/system.h"
#include "game/undo.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/file_dialog.h"
#include "window/popup_dialog.h"
#include "window/city.h"
#include "window/main_menu.h"
#include "window/mission_selection.h"
#include "window/plain_message_dialog.h"

static void button_click(const generic_button *button);

static unsigned int focus_button_id;

static generic_button buttons[] = {
        {192, 100, 192, 25, button_click, 0, 1},
        {192, 140, 192, 25, button_click, 0, 2},
        {192, 180, 192, 25, button_click, 0, 3},
        {192, 220, 192, 25, button_click, 0, 4},
        {192, 260, 192, 25, button_click, 0, 5},
        {192, 300, 192, 25, button_click, 0, 6},
        {192, 340, 192, 25, button_click, 0, 7},
};

#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(160, 44, 16, 22);

    for (size_t i = 0; i < MAX_BUTTONS; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, focus_button_id == i + 1 ? 1 : 0);
    }

    text_draw_centered(translation_for(TR_LABEL_PAUSE_MENU), 192, 58, 192, FONT_LARGE_BLACK, 0);
    lang_text_draw_centered(13, 5, 192, 108, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(1, 2, 192, 148, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(1, 3, 192, 188, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(1, 4, 192, 228, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(1, 6, 192, 268, 192, FONT_NORMAL_GREEN);
    text_draw_centered(translation_for(TR_BUTTON_BACK_TO_MAIN_MENU), 192, 308, 192, FONT_NORMAL_GREEN, 0);
    lang_text_draw_centered(1, 5, 192, 348, 192, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    }
    if (h->save_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
    }
}

static void replay_map_confirmed(int confirmed, int checked)
{
    if (!confirmed) {
        return;
    }
    if (!game_campaign_is_active()) {
        if (!game_file_start_scenario_by_name(scenario_name())) {
            window_plain_message_dialog_show_with_extra(TR_REPLAY_MAP_NOT_FOUND_TITLE,
                TR_REPLAY_MAP_NOT_FOUND_MESSAGE, 0, scenario_name());
        } else {
            window_city_show();
        }
    } else {
        int mission_id = game_campaign_is_original() ? scenario_campaign_mission() : 0;
        setting_set_personal_savings_for_mission(mission_id, scenario_starting_personal_savings());
        scenario_save_campaign_player_name();
        window_mission_selection_show_again();
    }
}

static void main_menu_confirmed(int confirmed, int checked)
{
    if (confirmed) {
        building_construction_clear_type();
        game_undo_disable();
        game_state_reset_overlay();
        window_main_menu_show(1);
    }
}

static void confirm_exit(int accepted, int checked)
{
    if (accepted) {
        system_exit();
    }
}

static void button_click(const generic_button *button)
{
    int type = button->parameter1;
    if (type == 1) {
        window_go_back();
    } else if (type == 2) {
        window_popup_dialog_show_confirmation(lang_get_string(1, 2), 0, 0, replay_map_confirmed);
    } else if (type == 3) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    } else if (type == 4) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
    } else if (type == 5) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_DELETE);
    } else if (type == 6) {
        window_popup_dialog_show_confirmation(translation_for(TR_BUTTON_BACK_TO_MAIN_MENU), 0, 0, main_menu_confirmed);
    } else if (type == 7) {
        window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
    }
}

void window_city_pause_menu_show(void)
{
    window_type window = {
            WINDOW_CITY_MAIN_MENU,
            window_draw_underlying_window,
            draw_foreground,
            handle_input
    };
    window_show(&window);
}
