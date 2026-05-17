#include "actions.h"
#include "app.h"
#include "logger.h"
#include "process.h"
#include "profiles.h"
#include "ui.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void ensure_logs_dir(void)
{
    mkdir("logs", 0755);
}

int main(void)
{
    setlocale(LC_ALL, "");

    ensure_logs_dir();

    AppState app;
    memset(&app, 0, sizeof(app));

    app.pipe_fd[0] = -1;
    app.pipe_fd[1] = -1;
    app.active_section = SECTION_PROFILES;
    app.selected_profile = 0;
    app.selected_action = ACTION_INSTALL;

    ui_init(&app);

    if (!logger_init(&app, "logs/sysopenmath.log"))
    {
        endwin();
        return 1;
    }

    log_info(&app, "SysOpenMath запущен.");
    load_profiles(&app, "profiles");

    while (1)
    {
        check_process(&app);
        read_process_output(&app);
        ui_draw(&app);

        int ch = getch();
        if (ch == ERR)
        {
            continue;
        }

        if (ch == KEY_RESIZE)
        {
            ui_recreate(&app);
            log_info(&app, "Размер окна изменён.");
            continue;
        }

        ui_handle_input(&app, ch);
    }

    logger_close(&app);
    ui_destroy(&app);
    return 0;
}