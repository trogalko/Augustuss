#ifndef GRAPHICS_WINDOW_H
#define GRAPHICS_WINDOW_H

#include "graphics/tooltip.h"
#include "input/hotkey.h"
#include "input/mouse.h"

typedef enum {
    WINDOW_LOGO,
    WINDOW_MAIN_MENU,
    WINDOW_CONFIG,
    WINDOW_HOTKEY_CONFIG,
    WINDOW_HOTKEY_EDITOR,
    WINDOW_SELECT_CAMPAIGN,
    WINDOW_MISSION_LIST,
    WINDOW_CCK_SELECTION,
    WINDOW_FILE_DIALOG,
    WINDOW_POPUP_DIALOG,
    WINDOW_PLAIN_MESSAGE_DIALOG,
    WINDOW_INTRO_VIDEO,
    // mission start/end
    WINDOW_INTERMEZZO,
    WINDOW_MISSION_SELECTION,
    WINDOW_MISSION_BRIEFING,
    WINDOW_VICTORY_DIALOG,
    WINDOW_VIDEO,
    WINDOW_MISSION_END,
    // city
    WINDOW_CITY,
    WINDOW_CITY_MILITARY,
    WINDOW_TOP_MENU,
    WINDOW_OVERLAY_MENU,
    WINDOW_MILITARY_MENU,
    WINDOW_BUILD_MENU,
    WINDOW_SLIDING_SIDEBAR,
    WINDOW_MESSAGE_DIALOG,
    WINDOW_MESSAGE_LIST,
    WINDOW_BUILDING_INFO,
    WINDOW_CITY_MAIN_MENU,
    WINDOW_HOLD_GAMES,
    WINDOW_RACE_BET,
    // advisors and dialogs
    WINDOW_ADVISORS,
    WINDOW_LABOR_PRIORITY,
    WINDOW_SET_SALARY,
    WINDOW_DONATE_TO_CITY,
    WINDOW_GIFT_TO_EMPEROR,
    WINDOW_TRADE_PRICES,
    WINDOW_RESOURCE_SETTINGS,
    WINDOW_HOLD_FESTIVAL,
    // empire and dialog
    WINDOW_EMPIRE,
    WINDOW_TRADE_OPENED,
    // options dialogs
    WINDOW_DIFFICULTY_OPTIONS,
    WINDOW_DISPLAY_OPTIONS,
    WINDOW_SOUND_OPTIONS,
    WINDOW_SPEED_OPTIONS,
    // utility windows
    WINDOW_SELECT_LIST,
    WINDOW_NUMERIC_INPUT,
    // editor
    WINDOW_EDITOR_MAP,
    WINDOW_EDITOR_TOP_MENU,
    WINDOW_EDITOR_BUILD_MENU,
    WINDOW_EDITOR_EMPIRE,
    WINDOW_EDITOR_ATTRIBUTES,
    WINDOW_EDITOR_ALLOWED_BUILDINGS,
    WINDOW_EDITOR_INVASIONS,
    WINDOW_EDITOR_EDIT_INVASION,
    WINDOW_EDITOR_REQUESTS,
    WINDOW_EDITOR_EDIT_REQUEST,
    WINDOW_EDITOR_STARTING_CONDITIONS,
    WINDOW_EDITOR_START_YEAR,
    WINDOW_EDITOR_SPECIAL_EVENTS,
    WINDOW_EDITOR_PRICE_CHANGES,
    WINDOW_EDITOR_EDIT_PRICE_CHANGE,
    WINDOW_EDITOR_DEMAND_CHANGES,
    WINDOW_EDITOR_EDIT_DEMAND_CHANGE,
    WINDOW_EDITOR_WIN_CRITERIA,
    WINDOW_EDITOR_SCENARIO_EVENTS,
    WINDOW_EDITOR_SCENARIO_EVENT_DETAILS,
    WINDOW_EDITOR_SCENARIO_ACTION_EDIT,
    WINDOW_EDITOR_SCENARIO_CONDITION_EDIT,
    WINDOW_EDITOR_SCENARIO_ACTION_TYPE,
    WINDOW_EDITOR_SCENARIO_CONDITION_TYPE,
    WINDOW_EDITOR_SELECT_CITY_TADE_ROUTE,
    WINDOW_EDITOR_SELECT_CITY_BY_TYPE,
    WINDOW_EDITOR_SELECT_SPECIAL_ATTRIBUTE_MAPPING,
    WINDOW_EDITOR_CUSTOM_MESSAGES,
    WINDOW_EDITOR_SELECT_CUSTOM_MESSAGE,
    WINDOW_EDITOR_CUSTOM_VARIABLES,
    // New window types
    WINDOW_ASSET_PREVIEWER,
    WINDOW_CUSTOM_MESSAGE,
    WINDOW_TEXT_INPUT,
    WINDOW_USER_PATH_SETUP
} window_id;

typedef struct {
    window_id id;
    void (*draw_background)(void);
    void (*draw_foreground)(void);
    void (*handle_input)(const mouse *m, const hotkeys *h);
    void (*get_tooltip)(tooltip_context *c);
    void (*on_return)(window_id from);
} window_type;

/**
 * Invalidates the window immediately, indicating that the current game state
 * requires a redraw before continuing
 */
void window_invalidate(void);

/**
 * Request a (soft) refresh of the window; does not invalidate the game state
 */
void window_request_refresh(void);

/**
 * Returns whether the window has been invalidated using `window_invalidate`
 */
int window_is_invalid(void);

void window_draw(int force);

void window_draw_underlying_window(void);

int window_is(window_id id);

void window_show(const window_type *window);

window_id window_get_id(void);

void window_go_back(void);

#endif // GRAPHICS_WINDOW_H
