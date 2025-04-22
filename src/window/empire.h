#ifndef WINDOW_EMPIRE_H
#define WINDOW_EMPIRE_H

#include "empire/object.h"

void window_empire_draw_border(const empire_object *border, int x_offset, int y_offset);

void window_empire_draw_trade_waypoints(const empire_object *trade_route, int x_offset, int y_offset);

void window_empire_draw_resource_shields(int trade_max, int x_offset, int y_offset);

void window_empire_show(void);

void window_empire_show_checked(void);

#endif // WINDOW_EMPIRE_H
