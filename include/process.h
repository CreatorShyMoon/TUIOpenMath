#ifndef PROCESS_H
#define PROCESS_H

#include "app.h"

int run_command_async(AppState *app, const char *cmd);
void read_process_output(AppState *app);
void check_process(AppState *app);
void terminate_running_process(AppState *app);

#endif