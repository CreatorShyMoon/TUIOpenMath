#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "app.h"

#include <stddef.h>

int state_build_show_state_command(char *out_cmd, size_t out_cmd_size);
int state_build_stop_jupyter_command(char *out_cmd, size_t out_cmd_size);

void state_clear_pending_update(AppState *app);
void state_set_pending_julia(AppState *app);
void state_set_pending_jupyter(AppState *app);
void state_set_pending_profile(AppState *app, const char *profile_name);

int state_apply_pending_update(AppState *app);

#endif