#ifndef APP_H
#define APP_H

#include <ncurses.h>
#include <stdio.h>
#include <sys/types.h>

#define MAX_PROFILES 64
#define MAX_NAME 128
#define MAX_DESC 256
#define MAX_PATH_LEN 512
#define MAX_CMD 16384
#define LOG_HEIGHT 12
#define ACTION_COUNT 31
#define ACTION_CATEGORY_COUNT 9

typedef enum AppSection
{
    SECTION_PROFILES = 0,
    SECTION_ACTIONS = 1,
    SECTION_LOG = 2
} AppSection;

typedef enum ActionType
{
    ACTION_INSTALL = 0,
    ACTION_UPDATE = 1,
    ACTION_DELETE_ENV = 2,
    ACTION_CHECK = 3,
    ACTION_RELOAD = 4,
    ACTION_INSTALL_JULIA_LTS = 5,
    ACTION_INSTALL_JULIA_STABLE = 6,
    ACTION_SHOW_JULIA_VERSION = 7,
    ACTION_INSTALL_JUPYTER = 8,
    ACTION_SHOW_JUPYTER_VERSION = 9,
    ACTION_SHOW_JUPYTER_KERNELS = 10,
    ACTION_START_JUPYTERLAB = 11,
    ACTION_REGISTER_PYTHON_KERNEL = 12,
    ACTION_REGISTER_JULIA_KERNEL = 13,
    ACTION_STOP_JUPYTERLAB = 14,
    ACTION_SHOW_STATE = 15,
    ACTION_REMOVE_JULIA = 16,
    ACTION_REMOVE_JUPYTER = 17,
    ACTION_CLEAR_KERNELS = 18,
    ACTION_CLEAR_STATE = 19,
    ACTION_CREATE_WORKSPACE = 20,
    ACTION_REGISTER_BASH_KERNEL = 21,
    ACTION_REGISTER_OCTAVE_KERNEL = 22,
    ACTION_FULL_SETUP_SELECTED = 23,
    ACTION_INSTALL_OCTAVE = 24,
    ACTION_REMOVE_OCTAVE = 25,
    ACTION_INSTALL_SCILAB = 26,
    ACTION_REMOVE_SCILAB =  27,
    ACTION_REGISTER_SCILAB_KERNEL = 28,
    ACTION_RESTART_JUPYTERLAB = 29,
    ACTION_EXIT = 30
} ActionType;

typedef enum PendingStateUpdateType
{
    PENDING_STATE_NONE = 0,
    PENDING_STATE_JULIA = 1,
    PENDING_STATE_JUPYTER = 2,
    PENDING_STATE_PROFILE = 3
} PendingStateUpdateType;

typedef struct Profile
{
    char name[MAX_NAME];
    char description[MAX_DESC];
    char path[MAX_PATH_LEN];
    int is_system_profile;
    int is_locked;
} Profile;

typedef struct AppState
{
    WINDOW *profiles_win;
    WINDOW *actions_win;
    WINDOW *log_win;
    WINDOW *status_win;

    int term_h;
    int term_w;

    int busy;
    pid_t current_pid;
    int pipe_fd[2];

    Profile profiles[MAX_PROFILES];
    int profile_count;

    int selected_profile;
    int selected_action;
    AppSection active_section;

    int confirm_delete;
    int confirm_exit;

    FILE *log_file;

    PendingStateUpdateType pending_state_update;
    char pending_profile_name[MAX_NAME];

    int profile_scroll;
    int log_scroll;

    int action_menu_level;      /* 0 = категории, 1 = действия */
    int selected_category;
    int selected_subitem;
    int action_category_scroll;
    int action_subitem_scroll;
} AppState;

#endif