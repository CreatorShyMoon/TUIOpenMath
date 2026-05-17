#ifndef UI_H
#define UI_H

#include "app.h"

void ui_init(AppState *app);
void ui_recreate(AppState *app);
void ui_destroy(AppState *app);
void ui_draw(AppState *app);
void ui_handle_input(AppState *app, int ch);
const char *ui_action_label(int action);

#endif