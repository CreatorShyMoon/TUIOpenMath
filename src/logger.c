#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void current_time_string(char *buf, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if (!tm_info)
    {
        snprintf(buf, size, "unknown-time");
        return;
    }

    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

int logger_init(AppState *app, const char *path)
{
    if (!app || !path)
    {
        return 0;
    }

    app->log_file = fopen(path, "a");
    return app->log_file != NULL;
}

void logger_close(AppState *app)
{
    if (!app || !app->log_file)
    {
        return;
    }

    fclose(app->log_file);
    app->log_file = NULL;
}

static void write_log_window(AppState *app, const char *line)
{
    if (!app || !app->log_win || !line)
    {
        return;
    }

    int max_y = getmaxy(app->log_win);
    int cur_y = getcury(app->log_win);
    int cur_x = getcurx(app->log_win);

    if (cur_y <= 0 || cur_x <= 0)
    {
        wmove(app->log_win, 1, 1);
    }

    wprintw(app->log_win, "%s\n", line);

    if (getcury(app->log_win) >= max_y - 1)
    {
        wscrl(app->log_win, 1);
        wmove(app->log_win, max_y - 2, 1);
    }

    box(app->log_win, 0, 0);
    mvwprintw(app->log_win, 0, 2, " Журнал ");
    wrefresh(app->log_win);
}

void log_message(AppState *app, const char *level, const char *fmt, ...)
{
    if (!app || !level || !fmt)
    {
        return;
    }

    char message[2048];
    char final_line[2300];
    char ts[64];

    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    current_time_string(ts, sizeof(ts));
    snprintf(final_line, sizeof(final_line), "[%s] [%s] %s", ts, level, message);

    if (app->log_file)
    {
        fprintf(app->log_file, "%s\n", final_line);
        fflush(app->log_file);
    }

    write_log_window(app, final_line);
}

void log_info(AppState *app, const char *fmt, ...)
{
    char message[2048];

    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    log_message(app, "INFO", "%s", message);
}

void log_warn(AppState *app, const char *fmt, ...)
{
    char message[2048];

    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    log_message(app, "WARN", "%s", message);
}

void log_error(AppState *app, const char *fmt, ...)
{
    char message[2048];

    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    log_message(app, "ERROR", "%s", message);
}