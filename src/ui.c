#include "ui.h"
#include "actions.h"
#include "logger.h"
#include "process.h"
#include "profiles.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct ActionCategory
{
    const char *label;
    const int *actions;
    int action_count;
} ActionCategory;


static const char *SPLASH_LOGO[] = {
"     ⣀⣀⡀                              ⢀⣤⣶⣾⣿⣿⣷⣦⡀⡀ ",
"   ⢠⣾⠉⠿⠟                              ⢿⣿⣿⣿⣿⣿⣿⣿⣿⣞⢷⣄ ",
"   ⣸⣿                                    ⠙⠛⠿⠿⠿⢿⣿⣿⣿⣎⢿⣷⣄  ",
"   ⣿⣿                                             ⠛⠎⣿⣿⣧⡀  ",
"   ⣿⣿     ⢀⣠⣤⣤⣀⡀     ⢀⣀⣤⣤⣄⣀     ⢀           ⡀        ⡸⣿⣿⣷..  ",
"   ⣿⣿   ⣤⣾⣿⠿⠿⠿⠿⣿⣷⠄ ⣠⣾⣿⠿⠿⠿⠿⣿⣿⣦⡀  ⢸⣧⡀       ⢀⣼⡇        ⠈⢿⣿⣿⣿⣿⣿⣠⡀",
"   ⣿⣿  ⣾⣿⠟⠁    ⠈⠁ ⣼⣿⡟⣁⣤⣤⣤⣤⣤⡙⣿⣿⡄ ⢸⣿⣷⡄     ⢠⣾⣿⡇         ⠘⣿⢻⣿⣿⣿⡏⣷⡀",
"   ⣿⣿ ⢸⣿⡟        ⢰⣿⣿⠈⠁⢸⡇ ⣿  ⢸⣿⣇ ⢸⣿⡿⣿⣆   ⣰⣿⣿⣿⡇          ⣿⣾⣿⣿⣿⣿⣿⣧",
"   ⣿⣿ ⠸⣿⣷⡀       ⠈⣿⣿⡀⢀⣼⠃ ⣿⣠⠄⣸⣿⡇ ⢸⣿⡇⠹⣿⣧⣀⣼⣿⠏⢸⣿⡇          ⢻⣿⣿⣿⣿⣿⣿⣿⡆",
"   ⣿⣿  ⠹⣿⣷⣄⡀  ⢀⣠⣦ ⠙⣿⣷⣬⣀  ⢀⣡⣾⣿⠟  ⢸⣿⡇ ⠘⣿⣿⣿⠃ ⢸⣿⡇          ⣾⣿⣿⣿⣿⡏⣿⣿⣧",
"   ⣿⣿   ⠈⠛⠿⣿⣿⣿⣿⠿⠛⠁ ⠈⠛⠿⣿⣿⣿⣿⡿⠟⠉   ⢸⣿⡇  ⠈⠿⠁  ⢸⣿⡇        ⣠⣾⣿⣿⣿⣿⣿⢡⣿⣿⣿",
"   ⣿⣿                                    ⢀⣀⡀      ⢀⣴⣿⣿⣿⣿⣿⣿⢃⣾⣿⣿⣿",
"   ⣿⡏                 ⢄                ⢠⣶⣿⣿⡇   ⣠⣾⣿⣿⣿⣿⣿⣿⣿⣣⣾⣿⣿⣿⡿",
"⣤⣄ ⣿⠃                 ⠈⣆              ⢠⣿⣿⣿⣿⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠇",
"⠙⠛⠛⠁                   ⠸⡆            ⣀⣬⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟",
"                         ⢹⡄       ⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠁",
"                          ⢻⣄ ⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠁",
"                           ⠙⢷⣤⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡻⣿⣿⣿⣿⣿⣿⣟⠋",
"                              ⢿⣿⣿⣿⣿⣿⣿⣿⣿⡍⣹⣿⣿⣿⣿⣿⣧⠹⣿⣿⣿⣿⣿⣿⣿⣦",
"                              ⠈⠳⣍⠻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄⠻⠛⠛⠛⠛⠛⠛⠁",
"                                 ⠙⠲⠬⣿⣿⣿⣿⣿⣿⡿⠿⠛⠋⠁",
"                                        ⠈⠙⠳⢦⣤⣤⣤⣤⣤⡤⠤"
};

static const int SPLASH_LOGO_LINES =

    sizeof(SPLASH_LOGO) / sizeof(SPLASH_LOGO[0]);

static void draw_splash_screen(void)

{

    erase();

    int max_y, max_x;

    getmaxyx(stdscr, max_y, max_x);

    int logo_width = 78;

    int min_w = logo_width + 4;

    int min_h = SPLASH_LOGO_LINES + 8;

    if (max_x < min_w || max_y < min_h)

    {

        erase();

        mvprintw(max_y / 2 - 1, max_x > 12 ? (max_x - 11) / 2 : 0, "SysOpenMath");

        mvprintw(max_y / 2 + 1, max_x > 30 ? (max_x - 29) / 2 : 0, "Рекомендуемый размер окна: 140x40");

        mvprintw(max_y / 2 + 2, max_x > 27 ? (max_x - 26) / 2 : 0, "Нажмите на любую клавишу, чтобы продолжить");

        refresh();

        flushinp();

        timeout(-1);

        getch();

        return;

    }

    int start_y = (max_y - SPLASH_LOGO_LINES) / 2 - 2;

    int start_x = (max_x - logo_width) / 2;

    if (start_y < 1) start_y = 1;

    if (start_x < 1) start_x = 1;

    if (has_colors())

    {

        attron(COLOR_PAIR(2));

    }

    else

    {

        attron(A_BOLD);

    }

    for (int i = 0; i < SPLASH_LOGO_LINES; i++)

    {

        if (start_y + i >= max_y - 1)

        {

            break;

        }

        mvprintw(start_y + i, start_x, "%s", SPLASH_LOGO[i]);

    }

    if (has_colors())

    {

        attroff(COLOR_PAIR(2));

    }

    else

    {

        attroff(A_BOLD);

    }

    mvprintw(start_y + SPLASH_LOGO_LINES + 2, start_x + 18, "SysOpenMath");

    mvprintw(start_y + SPLASH_LOGO_LINES + 3, start_x + 2, "Менеджер открытой математической стреды");

    mvprintw(start_y + SPLASH_LOGO_LINES + 5, start_x + 8, "Нажмите на любую клавишу, чтобы продолжить...");

    refresh();

    flushinp();

    timeout(-1);

    getch();

}


static const char *ACTION_LABELS[ACTION_COUNT] = {
    "Установить профиль",
    "Обновить профиль",
    "Удалить Python env",
    "Проверить компоненты",
    "Перезагрузить профили",
    "Установить Julia LTS",
    "Установить Julia Stable",
    "Показать версию Julia",
    "Установить Jupyter",
    "Показать версию Jupyter",
    "Показать kernels Jupyter",
    "Запустить JupyterLab",
    "Зарегистрировать Python kernel",
    "Зарегистрировать Julia kernel",
    "Остановить JupyterLab",
    "Показать состояние SysOpenMath",
    "Удалить Julia",
    "Удалить Jupyter",
    "Очистить kernels",
    "Очистить state",
    "Создать workspace",
    "Зарегистрировать Bash kernel",
    "Зарегистрировать Octave kernel",
    "Подготовить выбранный профиль полностью",
    "Установить Octave",
    "Удалить Octave",
    "Установить Scilab",
    "Удалить Scilab",
    "Зарегистрировать Scilab kernel",
    "Перезапустить JupyterLab",
    "Выход"
};

static const int PROFILE_ACTIONS[] = {
    ACTION_INSTALL,
    ACTION_UPDATE,
    ACTION_DELETE_ENV,
    ACTION_RELOAD
};

static const int JULIA_ACTIONS[] = {
    ACTION_INSTALL_JULIA_LTS,
    ACTION_INSTALL_JULIA_STABLE,
    ACTION_SHOW_JULIA_VERSION,
    ACTION_REMOVE_JULIA
};

static const int JUPYTER_ACTIONS[] = {
    ACTION_INSTALL_JUPYTER,
    ACTION_SHOW_JUPYTER_VERSION,
    ACTION_SHOW_JUPYTER_KERNELS,
    ACTION_START_JUPYTERLAB,
    ACTION_STOP_JUPYTERLAB,
    ACTION_RESTART_JUPYTERLAB,
    ACTION_REMOVE_JUPYTER
};

static const int KERNEL_ACTIONS[] = {
    ACTION_REGISTER_PYTHON_KERNEL,
    ACTION_REGISTER_JULIA_KERNEL,
    ACTION_REGISTER_BASH_KERNEL,
    ACTION_REGISTER_OCTAVE_KERNEL,
    ACTION_REGISTER_SCILAB_KERNEL,
    ACTION_CLEAR_KERNELS
};

static const int WORKSPACE_ACTIONS[] = {
    ACTION_CREATE_WORKSPACE
};

static const int DIAGNOSTIC_ACTIONS[] = {
    ACTION_CHECK,
    ACTION_SHOW_STATE,
    ACTION_CLEAR_STATE
};
static const int QUICK_SETUP_ACTIONS[] = {
    ACTION_FULL_SETUP_SELECTED
};

static const int SYSTEM_DEP_ACTIONS[] = {
    ACTION_INSTALL_SCILAB,
    ACTION_REMOVE_SCILAB,
    ACTION_INSTALL_OCTAVE,
    ACTION_REMOVE_OCTAVE
};

static const int SYSTEM_ACTIONS[] = {
    ACTION_EXIT
};

static const ActionCategory ACTION_CATEGORIES[ACTION_CATEGORY_COUNT] = {
    { "Быстрый старт", QUICK_SETUP_ACTIONS, (int)(sizeof(QUICK_SETUP_ACTIONS) / sizeof(QUICK_SETUP_ACTIONS[0])) },
    { "Профили",      PROFILE_ACTIONS,    (int)(sizeof(PROFILE_ACTIONS) / sizeof(PROFILE_ACTIONS[0])) },
    { "Julia",        JULIA_ACTIONS,      (int)(sizeof(JULIA_ACTIONS) / sizeof(JULIA_ACTIONS[0])) },
    { "Jupyter",      JUPYTER_ACTIONS,    (int)(sizeof(JUPYTER_ACTIONS) / sizeof(JUPYTER_ACTIONS[0])) },
    { "Системные зависимости", SYSTEM_DEP_ACTIONS, (int)(sizeof(SYSTEM_DEP_ACTIONS) / sizeof(SYSTEM_DEP_ACTIONS[0])) },
    { "Kernels",      KERNEL_ACTIONS,     (int)(sizeof(KERNEL_ACTIONS) / sizeof(KERNEL_ACTIONS[0])) },
    { "Workspace",    WORKSPACE_ACTIONS,  (int)(sizeof(WORKSPACE_ACTIONS) / sizeof(WORKSPACE_ACTIONS[0])) },
    { "Диагностика",  DIAGNOSTIC_ACTIONS, (int)(sizeof(DIAGNOSTIC_ACTIONS) / sizeof(DIAGNOSTIC_ACTIONS[0])) },
    { "Выход",        SYSTEM_ACTIONS,     (int)(sizeof(SYSTEM_ACTIONS) / sizeof(SYSTEM_ACTIONS[0])) }
};

static char *dup_string_local(const char *src)
{
    if (!src)
    {
        return NULL;
    }

    size_t len = strlen(src);
    char *copy = malloc(len + 1);
    if (!copy)
    {
        return NULL;
    }

    memcpy(copy, src, len + 1);
    return copy;
}

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value)
    {
        return min_value;
    }

    if (value > max_value)
    {
        return max_value;
    }

    return value;
}


static int current_action_id(const AppState *app)
{
    if (!app)
    {
        return ACTION_INSTALL;
    }

    if (app->action_menu_level == 0)
    {
        const ActionCategory *category = &ACTION_CATEGORIES[app->selected_category];
        if (category->action_count > 0)
        {
            return category->actions[0];
        }

        return ACTION_INSTALL;
    }

    const ActionCategory *category = &ACTION_CATEGORIES[app->selected_category];
    if (app->selected_subitem >= 0 && app->selected_subitem < category->action_count)
    {
        return category->actions[app->selected_subitem];
    }

    return ACTION_INSTALL;
}

const char *ui_action_label(int action)
{
    if (action < 0 || action >= ACTION_COUNT)
    {
        return "Неизвестно";
    }

    return ACTION_LABELS[action];
}

static void destroy_window_safe(WINDOW **win)
{
    if (win && *win)
    {
        delwin(*win);
        *win = NULL;
    }
}

static void create_windows(AppState *app)
{
    app->term_h = LINES;
    app->term_w = COLS;

    int status_h = 1;
    int log_h = LOG_HEIGHT;
    int top_h = app->term_h - status_h - log_h;

    if (top_h < 8)
    {
        top_h = 8;
        log_h = app->term_h - status_h - top_h;
        if (log_h < 4)
        {
            log_h = 4;
        }
    }

    int profiles_w = app->term_w / 2;
    int actions_w = app->term_w - profiles_w;

    app->profiles_win = newwin(top_h, profiles_w, 0, 0);
    app->actions_win  = newwin(top_h, actions_w, 0, profiles_w);
    app->log_win      = newwin(log_h, app->term_w, top_h, 0);
    app->status_win   = newwin(status_h, app->term_w, top_h + log_h, 0);

    scrollok(app->log_win, FALSE);
    keypad(stdscr, TRUE);
    
    draw_splash_screen();
timeout(100);
}
static void request_terminal_size(void)

{

    printf("\033[8;40;140t");

    fflush(stdout);

}
void ui_init(AppState *app)
{
    request_terminal_size();

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    draw_splash_screen();

    if (has_colors())
    {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_BLACK, COLOR_CYAN);
        init_pair(2, COLOR_YELLOW, -1);
    }

    create_windows(app);
}

void ui_recreate(AppState *app)
{
    destroy_window_safe(&app->profiles_win);
    destroy_window_safe(&app->actions_win);
    destroy_window_safe(&app->log_win);
    destroy_window_safe(&app->status_win);

    endwin();
    refresh();
    clear();

    create_windows(app);
}

void ui_destroy(AppState *app)
{
    terminate_running_process(app);

    destroy_window_safe(&app->profiles_win);
    destroy_window_safe(&app->actions_win);
    destroy_window_safe(&app->log_win);
    destroy_window_safe(&app->status_win);

    endwin();
}

static void draw_profiles(AppState *app)
{
    werase(app->profiles_win);
    box(app->profiles_win, 0, 0);
    mvwprintw(app->profiles_win, 0, 2, " Профили ");

    int max_y = getmaxy(app->profiles_win);
    int visible_rows = max_y - 6;
    if (visible_rows < 1)
    {
        visible_rows = 1;
    }

    if (app->profile_count == 0)
    {
        mvwprintw(app->profiles_win, 2, 2, "Нет профилей.");
    }
    else
    {
        app->selected_profile = clamp_int(app->selected_profile, 0, app->profile_count - 1);

        if (app->selected_profile < app->profile_scroll)
        {
            app->profile_scroll = app->selected_profile;
        }
        if (app->selected_profile >= app->profile_scroll + visible_rows)
        {
            app->profile_scroll = app->selected_profile - visible_rows + 1;
        }
        app->profile_scroll = clamp_int(app->profile_scroll, 0, app->profile_count - 1);

        int row = 2;
        for (int i = app->profile_scroll; i < app->profile_count && row < 2 + visible_rows; i++, row++)
        {
            int selected = (app->active_section == SECTION_PROFILES && i == app->selected_profile);

            if (selected && has_colors())
            {
                wattron(app->profiles_win, COLOR_PAIR(1));
            }
            else if (selected)
            {
                wattron(app->profiles_win, A_REVERSE);
            }

            mvwprintw(app->profiles_win, row, 2, "%s", app->profiles[i].name);

            if (selected && has_colors())
            {
                wattroff(app->profiles_win, COLOR_PAIR(1));
            }
            else if (selected)
            {
                wattroff(app->profiles_win, A_REVERSE);
            }
        }

        const Profile *profile = get_selected_profile(app);
        if (profile)
        {
            int panel_w = getmaxx(app->profiles_win) - 4;
            int y = max_y - 4;
            if (y > 2 && panel_w > 0)
            {
                mvwprintw(app->profiles_win, y - 1, 2, "Описание:");

                /* перенос описания по словам на две строки */
                const char *desc = profile->description;
                int len = (int)strlen(desc);

                if (len <= panel_w)
                {
                    mvwprintw(app->profiles_win, y, 2, "%s", desc);
                }
                else
                {
                    /* ищем последний пробел до конца первой строки */
                    int cut = panel_w;
                    while (cut > 0 && desc[cut] != ' ')
                        cut--;
                    if (cut == 0)
                        cut = panel_w;

                    mvwprintw(app->profiles_win, y, 2, "%.*s", cut, desc);

                    const char *rest = desc + cut + (desc[cut] == ' ' ? 1 : 0);
                    if (*rest != '\0')
                        mvwprintw(app->profiles_win, y + 1, 2, "%.*s", panel_w, rest);
                }
            }
        }
    }
    wnoutrefresh(app->profiles_win);
}

static void draw_actions_categories(AppState *app, int start_y, int max_y, int max_x)
{
    int visible_rows = max_y - start_y - 1;
    if (visible_rows < 1)
    {
        visible_rows = 1;
    }

    app->selected_category = clamp_int(app->selected_category, 0, ACTION_CATEGORY_COUNT - 1);

    if (app->selected_category < app->action_category_scroll)
    {
        app->action_category_scroll = app->selected_category;
    }
    if (app->selected_category >= app->action_category_scroll + visible_rows)
    {
        app->action_category_scroll = app->selected_category - visible_rows + 1;
    }
    app->action_category_scroll = clamp_int(app->action_category_scroll, 0, ACTION_CATEGORY_COUNT - 1);

    int row = start_y;
    for (int i = app->action_category_scroll; i < ACTION_CATEGORY_COUNT && row < max_y - 1; i++, row++)
    {
        int selected = (app->active_section == SECTION_ACTIONS && i == app->selected_category);

        if (selected && has_colors())
        {
            wattron(app->actions_win, COLOR_PAIR(1));
        }
        else if (selected)
        {
            wattron(app->actions_win, A_REVERSE);
        }

        mvwprintw(app->actions_win, row, 2, "[%s]", ACTION_CATEGORIES[i].label);

        if (selected && has_colors())
        {
            wattroff(app->actions_win, COLOR_PAIR(1));
        }
        else if (selected)
        {
            wattroff(app->actions_win, A_REVERSE);
        }
    }

    mvwprintw(app->actions_win, 1, 2, "Категории");
    mvwprintw(app->actions_win, max_y - 2, 2, "Enter/→ открыть");
    mvwprintw(app->actions_win, max_y - 2, max_x - 18, "Tab переключить");
}

static void draw_actions_subitems(AppState *app, int start_y, int max_y, int max_x)
{
    const ActionCategory *category = &ACTION_CATEGORIES[app->selected_category];

    int visible_rows = max_y - start_y - 1;
    if (visible_rows < 1)
    {
        visible_rows = 1;
    }

    if (category->action_count <= 0)
    {
        mvwprintw(app->actions_win, start_y, 2, "Нет действий.");
        return;
    }

    app->selected_subitem = clamp_int(app->selected_subitem, 0, category->action_count - 1);

    if (app->selected_subitem < app->action_subitem_scroll)
    {
        app->action_subitem_scroll = app->selected_subitem;
    }
    if (app->selected_subitem >= app->action_subitem_scroll + visible_rows)
    {
        app->action_subitem_scroll = app->selected_subitem - visible_rows + 1;
    }
    app->action_subitem_scroll = clamp_int(app->action_subitem_scroll, 0, category->action_count - 1);

    mvwprintw(app->actions_win, 1, 2, "%s", category->label);
    mvwprintw(app->actions_win, 2, 2, "← назад | Enter выполнить");

    int row = start_y;
    for (int i = app->action_subitem_scroll; i < category->action_count && row < max_y - 1; i++, row++)
    {
        int action_id = category->actions[i];
        int selected = (app->active_section == SECTION_ACTIONS && i == app->selected_subitem);

        if (selected && has_colors())
        {
            wattron(app->actions_win, COLOR_PAIR(1));
        }
        else if (selected)
        {
            wattron(app->actions_win, A_REVERSE);
        }

        mvwprintw(app->actions_win, row, 2, "%s", ui_action_label(action_id));

        if (selected && has_colors())
        {
            wattroff(app->actions_win, COLOR_PAIR(1));
        }
        else if (selected)
        {
            wattroff(app->actions_win, A_REVERSE);
        }
    }

    mvwprintw(app->actions_win, max_y - 2, max_x - 18, "Tab переключить");
}

static void draw_actions(AppState *app)
{
    werase(app->actions_win);
    box(app->actions_win, 0, 0);
    mvwprintw(app->actions_win, 0, 2, " Действия ");

    int max_y = getmaxy(app->actions_win);
    int max_x = getmaxx(app->actions_win);

    if (app->action_menu_level == 0)
    {
        draw_actions_categories(app, 2, max_y, max_x);
    }
    else
    {
        draw_actions_subitems(app, 4, max_y, max_x);
    }

    wnoutrefresh(app->actions_win);
}

static int read_log_lines(char *lines[], int max_lines)
{
    FILE *f = fopen("logs/sysopenmath.log", "r");
    if (!f)
    {
        return 0;
    }

    int count = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), f))
    {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
        {
            buffer[--len] = '\0';
        }

        char *copy = dup_string_local(buffer);
        if (!copy)
        {
            break;
        }

        if (count < max_lines)
        {
            lines[count++] = copy;
        }
        else
        {
            free(lines[0]);
            memmove(&lines[0], &lines[1], sizeof(char *) * (max_lines - 1));
            lines[max_lines - 1] = copy;
        }
    }

    fclose(f);
    return count;
}

static void free_log_lines(char *lines[], int count)
{
    for (int i = 0; i < count; i++)
    {
        free(lines[i]);
    }
}

static void draw_log(AppState *app)
{
    werase(app->log_win);
    box(app->log_win, 0, 0);
    mvwprintw(app->log_win, 0, 2, " Лог ");

    int max_y = getmaxy(app->log_win);
    int max_x = getmaxx(app->log_win);
    int visible_rows = max_y - 2;
    if (visible_rows < 1)
    {
        visible_rows = 1;
    }

    char *lines[512];
    int count = read_log_lines(lines, 512);

    if (count == 0)
    {
        mvwprintw(app->log_win, 1, 2, "Лог пуст.");
        wnoutrefresh(app->log_win);
        return;
    }

    int max_scroll = (count > visible_rows) ? (count - visible_rows) : 0;
    app->log_scroll = clamp_int(app->log_scroll, 0, max_scroll);

    int start = (max_scroll - app->log_scroll);
    if (start < 0)
    {
        start = 0;
    }

    int row = 1;
    for (int i = start; i < count && row < max_y - 1; i++, row++)
    {
        if (app->active_section == SECTION_LOG && row == 1)
        {
            wattron(app->log_win, COLOR_PAIR(2));
            mvwprintw(app->log_win, row, 1, "%.*s", max_x - 2, lines[i]);
            wattroff(app->log_win, COLOR_PAIR(2));
        }
        else
        {
            mvwprintw(app->log_win, row, 1, "%.*s", max_x - 2, lines[i]);
        }
    }

    mvwprintw(
        app->log_win,
        max_y - 1,
        max_x - 20,
        "PgUp/PgDn Home/End"
    );

    free_log_lines(lines, count);
    wnoutrefresh(app->log_win);
}

static void draw_status(AppState *app)
{
    werase(app->status_win);

    const char *section = "Профили";
    if (app->active_section == SECTION_ACTIONS)
    {
        section = "Действия";
    }
    else if (app->active_section == SECTION_LOG)
    {
        section = "Лог";
    }

    int action_id = current_action_id(app);
    app->selected_action = action_id;

    const char *action_label = ui_action_label(action_id);

    if (app->busy)
    {
        mvwprintw(
            app->status_win,
            0,
            0,
            "Раздел: %s | Действие: %s | Статус: выполняется... | Tab: секции | Enter: выбрать | ←: назад | q: выход",
            section,
            action_label
        );
    }
    else
    {
        mvwprintw(
            app->status_win,
            0,
            0,
            "Раздел: %s | Действие: %s | Готово | Tab: секции | Enter: выбрать | ←: назад | q: выход",
            section,
            action_label
        );
    }

    wnoutrefresh(app->status_win);
}

static void draw_dialogs(AppState *app)
{
    if (app->confirm_delete)
    {
        const Profile *profile = get_selected_profile(app);
        if (profile)
        {
            int h = 5;
            int w = 48;
            int y = (LINES - h) / 2;
            int x = (COLS - w) / 2;

            WINDOW *dialog = newwin(h, w, y, x);
            box(dialog, 0, 0);
            mvwprintw(dialog, 1, 2, "Удалить Python env envs/%s?", profile->name);
            mvwprintw(dialog, 2, 2, "y - да, n - нет");
            wrefresh(dialog);
            delwin(dialog);
        }
    }

    if (app->confirm_exit)
    {
        int h = 5;
        int w = 36;
        int y = (LINES - h) / 2;
        int x = (COLS - w) / 2;

        WINDOW *dialog = newwin(h, w, y, x);
        box(dialog, 0, 0);
        mvwprintw(dialog, 1, 2, "Выйти из программы?");
        mvwprintw(dialog, 2, 2, "y - да, n - нет");
        wrefresh(dialog);
        delwin(dialog);
    }
}

void ui_draw(AppState *app)
{
    draw_profiles(app);
    draw_actions(app);
    draw_log(app);
    draw_status(app);
    draw_dialogs(app);

    doupdate();
}

static void handle_confirm_delete(AppState *app, int ch)
{
    if (ch == 'y' || ch == 'Y')
    {
        app->confirm_delete = 0;
        perform_delete_environment(app);
    }
    else if (ch == 'n' || ch == 'N' || ch == 27)
    {
        app->confirm_delete = 0;
    }
}

static void handle_confirm_exit(AppState *app, int ch)
{
    if (ch == 'y' || ch == 'Y')
    {
        logger_close(app);
        ui_destroy(app);
        exit(0);
    }
    else if (ch == 'n' || ch == 'N' || ch == 27)
    {
        app->confirm_exit = 0;
    }
}

static void cycle_section(AppState *app)
{
    if (app->active_section == SECTION_PROFILES)
    {
        app->active_section = SECTION_ACTIONS;
    }
    else if (app->active_section == SECTION_ACTIONS)
    {
        app->active_section = SECTION_LOG;
    }
    else
    {
        app->active_section = SECTION_PROFILES;
    }
}

static void handle_profiles_input(AppState *app, int ch)
{
    if (app->profile_count <= 0)
    {
        return;
    }

    switch (ch)
    {
        case KEY_UP:
            if (app->selected_profile > 0)
            {
                app->selected_profile--;
            }
            break;

        case KEY_DOWN:
            if (app->selected_profile < app->profile_count - 1)
            {
                app->selected_profile++;
            }
            break;

        case KEY_HOME:
            app->selected_profile = 0;
            break;

        case KEY_END:
            app->selected_profile = app->profile_count - 1;
            break;

        default:
            break;
    }
}

static void handle_actions_input(AppState *app, int ch)
{
    if (app->action_menu_level == 0)
    {
        switch (ch)
        {
            case KEY_UP:
                if (app->selected_category > 0)
                {
                    app->selected_category--;
                }
                break;

            case KEY_DOWN:
                if (app->selected_category < ACTION_CATEGORY_COUNT - 1)
                {
                    app->selected_category++;
                }
                break;

            case KEY_HOME:
                app->selected_category = 0;
                break;

            case KEY_END:
                app->selected_category = ACTION_CATEGORY_COUNT - 1;
                break;

            case KEY_RIGHT:
            case 10:
            case KEY_ENTER:
                app->action_menu_level = 1;
                app->selected_subitem = 0;
                app->action_subitem_scroll = 0;
                break;

            default:
                break;
        }

        return;
    }

    const ActionCategory *category = &ACTION_CATEGORIES[app->selected_category];
    if (category->action_count <= 0)
    {
        return;
    }

    switch (ch)
    {
        case KEY_UP:
            if (app->selected_subitem > 0)
            {
                app->selected_subitem--;
            }
            break;

        case KEY_DOWN:
            if (app->selected_subitem < category->action_count - 1)
            {
                app->selected_subitem++;
            }
            break;

        case KEY_HOME:
            app->selected_subitem = 0;
            break;

        case KEY_END:
            app->selected_subitem = category->action_count - 1;
            break;

        case KEY_LEFT:
        case KEY_BACKSPACE:
        case 127:
            app->action_menu_level = 0;
            app->action_subitem_scroll = 0;
            break;

        case KEY_RIGHT:
        case 10:
        case KEY_ENTER:
            app->selected_action = category->actions[app->selected_subitem];
            perform_selected_action(app);
            break;

        default:
            break;
    }
}

static void handle_log_input(AppState *app, int ch)
{
    switch (ch)
    {
        case KEY_UP:
            app->log_scroll++;
            break;

        case KEY_DOWN:
            if (app->log_scroll > 0)
            {
                app->log_scroll--;
            }
            break;

        case KEY_PPAGE:
            app->log_scroll += 10;
            break;

        case KEY_NPAGE:
            app->log_scroll -= 10;
            if (app->log_scroll < 0)
            {
                app->log_scroll = 0;
            }
            break;

        case KEY_HOME:
            app->log_scroll = 1000000;
            break;

        case KEY_END:
            app->log_scroll = 0;
            break;

        default:
            break;
    }
}

void ui_handle_input(AppState *app, int ch)
{
    if (app->confirm_delete)
    {
        handle_confirm_delete(app, ch);
        return;
    }

    if (app->confirm_exit)
    {
        handle_confirm_exit(app, ch);
        return;
    }

    if (ch == '\t')
    {
        cycle_section(app);
        return;
    }

    if (ch == 'q' || ch == 'Q')
    {
        app->confirm_exit = 1;
        return;
    }

    if (app->busy)
    {
        if (app->active_section == SECTION_LOG)
        {
            handle_log_input(app, ch);
        }
        return;
    }

    switch (app->active_section)
    {
        case SECTION_PROFILES:
            handle_profiles_input(app, ch);
            break;

        case SECTION_ACTIONS:
            handle_actions_input(app, ch);
            break;

        case SECTION_LOG:
            handle_log_input(app, ch);
            break;

        default:
            break;
    }
}