#include "state_manager.h"
#include "julia_manager.h"
#include "jupyter_manager.h"
#include "logger.h"
#include "profile_config.h"
#include "profiles.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

FILE *popen(const char *command, const char *mode);
int pclose(FILE *stream);

static int ensure_dir_exists(const char *path)
{
    struct stat st;

    if (!path || path[0] == '\0')
    {
        return 0;
    }

    if (stat(path, &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }

    return mkdir(path, 0755) == 0;
}

static int write_text_file(const char *path, const char *text)
{
    if (!path || !text)
    {
        return 0;
    }

    FILE *f = fopen(path, "wb");
    if (!f)
    {
        return 0;
    }

    if (fputs(text, f) == EOF)
    {
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

static int capture_command_output(const char *command, char *buffer, size_t buffer_size)
{
    if (!command || !buffer || buffer_size == 0)
    {
        return 0;
    }

    buffer[0] = '\0';

    FILE *fp = popen(command, "r");
    if (!fp)
    {
        return 0;
    }

    size_t total = 0;
    while (fgets(buffer + total, (int)(buffer_size - total), fp))
    {
        total = strlen(buffer);
        if (total + 1 >= buffer_size)
        {
            break;
        }
    }

    int rc = pclose(fp);
    if (rc != 0)
    {
        return 0;
    }

    while (total > 0 &&
           (buffer[total - 1] == '\n' || buffer[total - 1] == '\r'))
    {
        buffer[--total] = '\0';
    }

    return buffer[0] != '\0';
}

static const Profile *find_profile_by_name(const AppState *app, const char *profile_name)
{
    if (!app || !profile_name)
    {
        return NULL;
    }

    for (int i = 0; i < app->profile_count; i++)
    {
        if (strcmp(app->profiles[i].name, profile_name) == 0)
        {
            return &app->profiles[i];
        }
    }

    return NULL;
}

static int state_update_julia(AppState *app)
{
    (void)app;

    if (!ensure_dir_exists("state"))
    {
        return 0;
    }

    char output[2048];
    if (!capture_command_output("tools/bin/julia --version", output, sizeof(output)))
    {
        return write_text_file("state/julia.version", "Julia not installed");
    }

    return write_text_file("state/julia.version", output);
}

static int state_update_jupyter(AppState *app)
{
    (void)app;

    if (!ensure_dir_exists("state"))
    {
        return 0;
    }

    char output[4096];
    if (!capture_command_output("tools/bin/jupyter --version", output, sizeof(output)))
    {
        return write_text_file("state/jupyter.version", "Jupyter not installed");
    }

    return write_text_file("state/jupyter.version", output);
}

static int state_update_profile(AppState *app, const char *profile_name)
{
    if (!app || !profile_name)
    {
        return 0;
    }

    if (!ensure_dir_exists("state") || !ensure_dir_exists("state/profiles"))
    {
        return 0;
    }

    const Profile *profile = find_profile_by_name(app, profile_name);
    if (!profile)
    {
        return 0;
    }

    ProfileConfig config;
    if (!profile_config_load(profile->name, &config))
    {
        return 0;
    }

    char state_path[512];
    int sw = snprintf(state_path, sizeof(state_path), "state/profiles/%s.state", profile->name);
    if (sw < 0 || (size_t)sw >= sizeof(state_path))
    {
        profile_config_free(&config);
        return 0;
    }

    FILE *f = fopen(state_path, "wb");
    if (!f)
    {
        profile_config_free(&config);
        return 0;
    }

    fprintf(f, "profile=%s\n", profile->name);
    fprintf(f, "python_env=envs/%s\n", profile->name);
    fprintf(f, "python_count=%zu\n", config.python.count);
    fprintf(f, "julia_count=%zu\n", config.julia.count);
    fprintf(f, "system_count=%zu\n", config.system.count);

    for (size_t i = 0; i < config.python.count; i++)
    {
        fprintf(f, "python[]=%s\n", config.python.items[i]);
    }

    for (size_t i = 0; i < config.julia.count; i++)
    {
        fprintf(f, "julia[]=%s\n", config.julia.items[i]);
    }

    for (size_t i = 0; i < config.system.count; i++)
    {
        fprintf(f, "system[]=%s\n", config.system.items[i]);
    }

    fclose(f);
    profile_config_free(&config);
    return 1;
}

void state_clear_pending_update(AppState *app)
{
    if (!app)
    {
        return;
    }

    app->pending_state_update = PENDING_STATE_NONE;
    app->pending_profile_name[0] = '\0';
}

void state_set_pending_julia(AppState *app)
{
    if (!app)
    {
        return;
    }

    app->pending_state_update = PENDING_STATE_JULIA;
    app->pending_profile_name[0] = '\0';
}

void state_set_pending_jupyter(AppState *app)
{
    if (!app)
    {
        return;
    }

    app->pending_state_update = PENDING_STATE_JUPYTER;
    app->pending_profile_name[0] = '\0';
}

void state_set_pending_profile(AppState *app, const char *profile_name)
{
    if (!app || !profile_name)
    {
        return;
    }

    app->pending_state_update = PENDING_STATE_PROFILE;
    snprintf(app->pending_profile_name, sizeof(app->pending_profile_name), "%s", profile_name);
}

int state_apply_pending_update(AppState *app)
{
    if (!app)
    {
        return 0;
    }

    int ok = 1;

    switch (app->pending_state_update)
    {
        case PENDING_STATE_JULIA:
            ok = state_update_julia(app);
            if (!ok)
            {
                log_warn(app, "Не удалось обновить state для Julia.");
            }
            break;

        case PENDING_STATE_JUPYTER:
            ok = state_update_jupyter(app);
            if (!ok)
            {
                log_warn(app, "Не удалось обновить state для Jupyter.");
            }
            break;

        case PENDING_STATE_PROFILE:
            ok = state_update_profile(app, app->pending_profile_name);
            if (!ok)
            {
                log_warn(app, "Не удалось обновить state для профиля %s.", app->pending_profile_name);
            }
            break;

        case PENDING_STATE_NONE:
        default:
            ok = 1;
            break;
    }

    state_clear_pending_update(app);
    return ok;
}

int state_build_show_state_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "echo \"[STATE] platform\"; "
        "if [ -f state/julia.version ]; then echo \"[STATE] julia\"; cat state/julia.version; else echo \"[STATE] julia state missing\"; fi; "
        "if [ -f state/jupyter.version ]; then echo \"[STATE] jupyter\"; cat state/jupyter.version; else echo \"[STATE] jupyter state missing\"; fi; "
        "echo \"[STATE] jupyterlab\"; "
        "pgrep -f \"tools/bin/jupyter-lab\" >/dev/null 2>&1 && echo \"[STATE] running\" || echo \"[STATE] stopped\"; "
        "echo \"[STATE] profiles\"; "
        "if ls state/profiles/*.state >/dev/null 2>&1; then "
        "  for f in state/profiles/*.state; do echo \"--- $f ---\"; cat \"$f\"; done; "
        "else "
        "  echo \"[STATE] no profile states\"; "
        "fi"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int state_build_stop_jupyter_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "if [ -f state/jupyter.pid ]; then "
        "  PID=$(cat state/jupyter.pid); "
        "  if kill \"$PID\" >/dev/null 2>&1; then "
        "    rm -f state/jupyter.pid; "
        "    echo \"[INFO] JupyterLab stopped (pid=$PID)\"; "
        "  else "
        "    rm -f state/jupyter.pid; "
        "    echo \"[WARN] JupyterLab pid file existed, but process was not running\"; "
        "  fi; "
        "else "
        "  echo \"[WARN] JupyterLab pid file not found\"; "
        "fi"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}