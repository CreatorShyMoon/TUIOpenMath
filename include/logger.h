#ifndef LOGGER_H
#define LOGGER_H

#include "app.h"

int logger_init(AppState *app, const char *path);
void logger_close(AppState *app);

void log_message(AppState *app, const char *level, const char *fmt, ...);
void log_info(AppState *app, const char *fmt, ...);
void log_warn(AppState *app, const char *fmt, ...);
void log_error(AppState *app, const char *fmt, ...);

#endif