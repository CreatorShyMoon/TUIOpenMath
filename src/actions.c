#include "actions.h"
#include "checks.h"
#include "cleanup_manager.h"
#include "install_steps.h"
#include "julia_manager.h"
#include "jupyter_manager.h"
#include "kernel_manager.h"
#include "logger.h"
#include "process.h"
#include "profile_config.h"
#include "profiles.h"
#include "state_manager.h"
#include "ui.h"
#include "workspace_manager.h"
#include "setup_manager.h"
#include "octave_manager.h"
#include "scilab_manager.h"

#include <stdio.h>
#include <string.h>

static int run_full_setup_for_profile(AppState *app, const char *profile_name)
{
    if (!app || !profile_name)
    {
        return 0;
    }

    const Profile *target = NULL;
    for (int i = 0; i < app->profile_count; i++)
    {
        if (strcmp(app->profiles[i].name, profile_name) == 0)
        {
            target = &app->profiles[i];
            break;
        }
    }

    if (!target)
    {
        log_error(app, "Профиль %s не найден.", profile_name);
        return 0;
    }

    char cmd[MAX_CMD];
    if (!setup_build_full_profile_command(target, cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить full setup для профиля %s.", profile_name);
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Полная подготовка профиля %s...", profile_name);
    return run_command_async(app, cmd);
}

int perform_full_setup_selected(AppState *app)
{
    const Profile *profile = get_selected_profile(app);
    if (!profile)
    {
        log_warn(app, "Профиль не выбран.");
        return 0;
    }

    return run_full_setup_for_profile(app, profile->name);
}

static int run_profile_install(AppState *app, int update_mode)
{
    const Profile *profile = get_selected_profile(app);
    if (!profile)
    {
        log_warn(app, "Профиль не выбран.");
        return 0;
    }

    ProfileConfig config;
    if (!profile_config_load(profile->name, &config))
    {
        log_error(app, "Не удалось загрузить или провалидировать профиль '%s'.", profile->name);
        return 0;
    }

    char cmd[MAX_CMD];
    int ok = build_install_command(profile, &config, update_mode, cmd, sizeof(cmd));
    profile_config_free(&config);

    if (!ok)
    {
        log_error(app, "Не удалось подготовить установочный сценарий для профиля '%s'.", profile->name);
        return 0;
    }

    state_set_pending_profile(app, profile->name);

    log_info(app, "%s профиля: %s", update_mode ? "Обновление" : "Установка", profile->name);
    return run_command_async(app, cmd);
}

int perform_install_profile(AppState *app)
{
    return run_profile_install(app, 0);
}

int perform_update_profile(AppState *app)
{
    return run_profile_install(app, 1);
}

int perform_delete_environment(AppState *app)
{
    const Profile *profile = get_selected_profile(app);
    if (!profile)
    {
        log_warn(app, "Профиль не выбран.");
        return 0;
    }

    char cmd[MAX_CMD];
    if (!build_delete_python_env_command(profile, cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить удаление Python env для профиля '%s'.", profile->name);
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Удаление Python env: envs/%s", profile->name);
    return run_command_async(app, cmd);
}

int perform_install_julia_lts(AppState *app)
{
    JuliaInstallRequest request;
    memset(&request, 0, sizeof(request));
    request.channel = JULIA_CHANNEL_LTS;

    char cmd[MAX_CMD];
    if (!julia_build_install_command(&request, cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить установку Julia LTS.");
        return 0;
    }

    state_set_pending_julia(app);

    log_info(app, "Установка Julia LTS...");
    return run_command_async(app, cmd);
}

int perform_install_julia_stable(AppState *app)
{
    JuliaInstallRequest request;
    memset(&request, 0, sizeof(request));
    request.channel = JULIA_CHANNEL_STABLE;

    char cmd[MAX_CMD];
    if (!julia_build_install_command(&request, cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить установку Julia Stable.");
        return 0;
    }

    state_set_pending_julia(app);

    log_info(app, "Установка Julia Stable...");
    return run_command_async(app, cmd);
}

int perform_show_julia_version(AppState *app)
{
    char cmd[MAX_CMD];
    if (!julia_build_version_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить проверку версии Julia.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Проверка версии Julia...");
    return run_command_async(app, cmd);
}

int perform_install_jupyter(AppState *app)
{
    char cmd[MAX_CMD];
    if (!jupyter_build_install_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить установку Jupyter.");
        return 0;
    }

    state_set_pending_jupyter(app);

    log_info(app, "Установка Jupyter...");
    return run_command_async(app, cmd);
}

int perform_show_jupyter_version(AppState *app)
{
    char cmd[MAX_CMD];
    if (!jupyter_build_version_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить проверку версии Jupyter.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Проверка версии Jupyter...");
    return run_command_async(app, cmd);
}

int perform_show_jupyter_kernels(AppState *app)
{
    char cmd[MAX_CMD];
    if (!jupyter_build_kernels_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить показ kernels Jupyter.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Проверка kernels Jupyter...");
    return run_command_async(app, cmd);
}

int perform_start_jupyterlab(AppState *app)
{
    char cmd[MAX_CMD];
    if (!jupyter_build_lab_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить запуск JupyterLab.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Запуск JupyterLab...");
    return run_command_async(app, cmd);
}

int perform_register_python_kernel(AppState *app)
{
    const Profile *profile = get_selected_profile(app);
    if (!profile)
    {
        log_warn(app, "Профиль не выбран.");
        return 0;
    }

    char cmd[MAX_CMD];
    if (!kernel_build_register_python_command(profile, cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить регистрацию Python kernel для '%s'.", profile->name);
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Регистрация Python kernel для профиля %s...", profile->name);
    return run_command_async(app, cmd);
}

int perform_register_julia_kernel(AppState *app)
{
    char cmd[MAX_CMD];
    if (!kernel_build_register_julia_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить регистрацию Julia kernel.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Регистрация Julia kernel...");
    return run_command_async(app, cmd);
}

int perform_stop_jupyterlab(AppState *app)
{
    char cmd[MAX_CMD];
    if (!state_build_stop_jupyter_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить остановку JupyterLab.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Остановка JupyterLab...");
    return run_command_async(app, cmd);
}

int perform_restart_jupyterlab(AppState *app)
{
    char stop_cmd[MAX_CMD / 2];
    char start_cmd[MAX_CMD / 2];

    if (!state_build_stop_jupyter_command(stop_cmd, sizeof(stop_cmd)))
    {
        log_error(app, "Не удалось подготовить остановку JupyterLab.");
        return 0;
    }

    if (!jupyter_build_lab_command(start_cmd, sizeof(start_cmd)))
    {
        log_error(app, "Не удалось подготовить запуск JupyterLab.");
        return 0;
    }

    char cmd[MAX_CMD];
    int written = snprintf(cmd, sizeof(cmd),
        "bash -lc 'set -euo pipefail; "
        "LOG_FILE=\"/tmp/sysopenmath_jupyterlab.log\"; "
        "if [ -f state/jupyter.pid ]; then "
        "  PID=$(cat state/jupyter.pid); "
        "  kill \"$PID\" >/dev/null 2>&1 || true; "
        "  rm -f state/jupyter.pid; "
        "  sleep 1; "
        "fi; "
        "if [ ! -x \"tools/bin/jupyter-lab\" ]; then echo \"[ERROR] JupyterLab не установлен\"; exit 1; fi; "
        "rm -f \"$LOG_FILE\"; "
        "nohup \"tools/bin/jupyter-lab\" --no-browser > \"$LOG_FILE\" 2>&1 < /dev/null & "
        "JPID=$!; mkdir -p state; echo \"$JPID\" > state/jupyter.pid; "
        "echo \"[INFO] JupyterLab restarted (pid=$JPID)\"; "
        "sleep 3; "
        "URL=$(grep -Eo \"http://(127\\.0\\.0\\.1|localhost):[0-9]+(/lab)?\\?token=[^[:space:]]+\" \"$LOG_FILE\" | head -n 1 || true); "
        "if [ -n \"$URL\" ]; then echo \"[INFO] Access URL: $URL\"; "
        "else echo \"[WARN] JupyterLab URL not found yet. Check /tmp/sysopenmath_jupyterlab.log\"; fi'"
    );

    if (written < 0 || (size_t)written >= sizeof(cmd))
    {
        log_error(app, "Команда перезапуска JupyterLab слишком длинная.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Перезапуск JupyterLab...");
    return run_command_async(app, cmd);
}

int perform_install_octave(AppState *app)
{
    char cmd[MAX_CMD];
    if (!octave_build_install_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить установку Octave.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Установка Octave...");
    return run_command_async(app, cmd);
}

int perform_remove_octave(AppState *app)
{
    char cmd[MAX_CMD];
    if (!octave_build_remove_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить удаление Octave.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Удаление Octave...");
    return run_command_async(app, cmd);
}

int perform_install_scilab(AppState *app)
{
    char cmd[MAX_CMD];
    if (!scilab_build_install_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить установку Scilab.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Установка Scilab...");
    return run_command_async(app, cmd);
}

int perform_remove_scilab(AppState *app)
{
    char cmd[MAX_CMD];
    if (!scilab_build_remove_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить удаление Scilab.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Удаление Scilab...");
    return run_command_async(app, cmd);
}

int perform_register_scilab_kernel(AppState *app)
{
    char cmd[MAX_CMD];
    if (!scilab_build_register_kernel_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить регистрацию Scilab kernel.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Регистрация Scilab kernel...");
    return run_command_async(app, cmd);
}

int perform_show_state(AppState *app)
{
    char cmd[MAX_CMD];
    if (!state_build_show_state_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить показ состояния.");
        return 0;
    }

    state_clear_pending_update(app);

    log_info(app, "Показ состояния SysOpenMath...");
    return run_command_async(app, cmd);
}

int perform_remove_julia(AppState *app)
{
    char cmd[MAX_CMD];
    if (!cleanup_build_remove_julia_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить удаление Julia.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Удаление Julia...");
    return run_command_async(app, cmd);
}

int perform_remove_jupyter(AppState *app)
{
    char cmd[MAX_CMD];
    if (!cleanup_build_remove_jupyter_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить удаление Jupyter.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Удаление Jupyter...");
    return run_command_async(app, cmd);
}

int perform_clear_kernels(AppState *app)
{
    char cmd[MAX_CMD];
    if (!cleanup_build_clear_kernels_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить очистку kernels.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Очистка SysOpenMath kernels...");
    return run_command_async(app, cmd);
}

int perform_clear_state(AppState *app)
{
    char cmd[MAX_CMD];
    if (!cleanup_build_clear_state_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить очистку state.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Очистка state...");
    return run_command_async(app, cmd);
}

int perform_create_workspace(AppState *app)
{
    const Profile *profile = get_selected_profile(app);
    if (!profile)
    {
        log_warn(app, "Профиль не выбран.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Создание workspace для профиля %s...", profile->name);
    return workspace_create(app, profile);
}

void perform_reload_profiles(AppState *app)
{
    if (!app)
    {
        return;
    }

    log_info(app, "Перезагрузка профилей...");

    if (!load_profiles(app, "profiles"))
    {
        log_error(app, "Не удалось перезагрузить профили.");
        return;
    }

    if (app->selected_profile < 0 || app->selected_profile >= app->profile_count)
    {
        app->selected_profile = 0;
    }

    app->profile_scroll = 0;

    log_info(app, "Профили успешно перезагружены. Загружено: %d", app->profile_count);
}

int perform_register_bash_kernel(AppState *app)
{
    char cmd[MAX_CMD];
    if (!kernel_build_register_bash_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить регистрацию Bash kernel.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Регистрация Bash kernel...");
    return run_command_async(app, cmd);
}

int perform_register_octave_kernel(AppState *app)
{
    char cmd[MAX_CMD];
    if (!kernel_build_register_octave_command(cmd, sizeof(cmd)))
    {
        log_error(app, "Не удалось подготовить регистрацию Octave kernel.");
        return 0;
    }

    state_clear_pending_update(app);
    log_info(app, "Регистрация Octave kernel...");
    return run_command_async(app, cmd);
}



void perform_selected_action(AppState *app)
{
    if (!app)
    {
        return;
    }

    switch (app->selected_action)
    {
        case ACTION_INSTALL:
            perform_install_profile(app);
            break;
        case ACTION_UPDATE:
            perform_update_profile(app);
            break;
        case ACTION_DELETE_ENV:
            app->confirm_delete = 1;
            break;
        case ACTION_CHECK:
            run_component_checks(app);
            break;
        case ACTION_RELOAD:
            perform_reload_profiles(app);
            break;
        case ACTION_INSTALL_JULIA_LTS:
            perform_install_julia_lts(app);
            break;
        case ACTION_INSTALL_JULIA_STABLE:
            perform_install_julia_stable(app);
            break;
        case ACTION_SHOW_JULIA_VERSION:
            perform_show_julia_version(app);
            break;
        case ACTION_INSTALL_JUPYTER:
            perform_install_jupyter(app);
            break;
        case ACTION_SHOW_JUPYTER_VERSION:
            perform_show_jupyter_version(app);
            break;
        case ACTION_SHOW_JUPYTER_KERNELS:
            perform_show_jupyter_kernels(app);
            break;
        case ACTION_START_JUPYTERLAB:
            perform_start_jupyterlab(app);
            break;
        case ACTION_REGISTER_PYTHON_KERNEL:
            perform_register_python_kernel(app);
            break;
        case ACTION_REGISTER_JULIA_KERNEL:
            perform_register_julia_kernel(app);
            break;
        case ACTION_STOP_JUPYTERLAB:
            perform_stop_jupyterlab(app);
            break;
        case ACTION_RESTART_JUPYTERLAB:
            perform_restart_jupyterlab(app);
            break;
        case ACTION_SHOW_STATE:
            perform_show_state(app);
            break;
        case ACTION_REMOVE_JULIA:
            perform_remove_julia(app);
            break;
        case ACTION_REMOVE_JUPYTER:
            perform_remove_jupyter(app);
            break;
        case ACTION_CLEAR_KERNELS:
            perform_clear_kernels(app);
            break;
        case ACTION_CLEAR_STATE:
            perform_clear_state(app);
            break;
        case ACTION_CREATE_WORKSPACE:
            perform_create_workspace(app);
            break;
        case ACTION_REGISTER_BASH_KERNEL:
            perform_register_bash_kernel(app);
            break;
        case ACTION_REGISTER_OCTAVE_KERNEL:
            perform_register_octave_kernel(app);
            break;
        case ACTION_EXIT:
            app->confirm_exit = 1;
            break;
        case ACTION_FULL_SETUP_SELECTED:
            perform_full_setup_selected(app);
            break;
        case ACTION_INSTALL_OCTAVE:
            perform_install_octave(app);
            break;
        case ACTION_REMOVE_OCTAVE:
            perform_remove_octave(app);
            break;
        case ACTION_INSTALL_SCILAB:
            perform_install_scilab(app);
            break;
        case ACTION_REMOVE_SCILAB:
            perform_remove_scilab(app);
            break;
        case ACTION_REGISTER_SCILAB_KERNEL:
            perform_register_scilab_kernel(app);
            break;
        default:
            break;
    }
}