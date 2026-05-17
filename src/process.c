#include "process.h"
#include "logger.h"
#include "state_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int kill(pid_t pid, int sig);

static void close_pipe_end(int *fd)
{
    if (fd && *fd != -1)
    {
        close(*fd);
        *fd = -1;
    }
}

static void reset_process_state(AppState *app)
{
    if (!app)
    {
        return;
    }

    app->busy = 0;
    app->current_pid = -1;
    close_pipe_end(&app->pipe_fd[0]);
    close_pipe_end(&app->pipe_fd[1]);
}

static void log_output_line(AppState *app, const char *line)
{
    if (!app || !line || line[0] == '\0')
    {
        return;
    }

    log_info(app, "%s", line);
}

static char g_pending_output[4096];
static size_t g_pending_output_len = 0;

static void append_and_log_lines(AppState *app, const char *chunk)
{
    if (!app || !chunk || chunk[0] == '\0')
    {
        return;
    }

    size_t chunk_len = strlen(chunk);
    size_t pos = 0;

    while (pos < chunk_len)
    {
        char ch = chunk[pos++];

        if (g_pending_output_len + 1 < sizeof(g_pending_output))
        {
            g_pending_output[g_pending_output_len++] = ch;
        }

        if (ch == '\n')
        {
            while (g_pending_output_len > 0 &&
                   (g_pending_output[g_pending_output_len - 1] == '\n' ||
                    g_pending_output[g_pending_output_len - 1] == '\r'))
            {
                g_pending_output_len--;
            }

            g_pending_output[g_pending_output_len] = '\0';

            if (g_pending_output_len > 0)
            {
                log_output_line(app, g_pending_output);
            }

            g_pending_output_len = 0;
        }
    }
}

static void flush_output_tail(AppState *app)
{
    if (!app)
    {
        return;
    }

    while (g_pending_output_len > 0 &&
           (g_pending_output[g_pending_output_len - 1] == '\n' ||
            g_pending_output[g_pending_output_len - 1] == '\r'))
    {
        g_pending_output_len--;
    }

    g_pending_output[g_pending_output_len] = '\0';

    if (g_pending_output_len > 0)
    {
        log_output_line(app, g_pending_output);
    }

    g_pending_output_len = 0;
}

static void drain_process_pipe(AppState *app, int stop_on_eagain)
{
    if (!app || app->pipe_fd[0] == -1)
    {
        return;
    }

    char buf[512];

    while (1)
    {
        ssize_t n = read(app->pipe_fd[0], buf, sizeof(buf) - 1);

        if (n > 0)
        {
            buf[n] = '\0';
            append_and_log_lines(app, buf);
            continue;
        }

        if (n == 0)
        {
            break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            if (stop_on_eagain)
            {
                break;
            }

            break;
        }

        log_error(app, "Ошибка чтения вывода процесса: %s", strerror(errno));
        break;
    }
}

int run_command_async(AppState *app, const char *cmd)
{
    if (!app || !cmd)
    {
        return 0;
    }

    if (app->busy)
    {
        log_warn(app, "Система занята. Дождитесь завершения текущей операции.");
        return 0;
    }

    app->pipe_fd[0] = -1;
    app->pipe_fd[1] = -1;
    g_pending_output_len = 0;

    if (pipe(app->pipe_fd) == -1)
    {
        log_error(app, "Ошибка pipe: %s", strerror(errno));
        state_clear_pending_update(app);
        return 0;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        log_error(app, "Ошибка fork: %s", strerror(errno));
        close_pipe_end(&app->pipe_fd[0]);
        close_pipe_end(&app->pipe_fd[1]);
        state_clear_pending_update(app);
        return 0;
    }

    if (pid == 0)
    {
        close_pipe_end(&app->pipe_fd[0]);

        if (dup2(app->pipe_fd[1], STDOUT_FILENO) == -1)
        {
            _exit(127);
        }

        if (dup2(app->pipe_fd[1], STDERR_FILENO) == -1)
        {
            _exit(127);
        }

        close_pipe_end(&app->pipe_fd[1]);

        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }

    close_pipe_end(&app->pipe_fd[1]);

    int flags = fcntl(app->pipe_fd[0], F_GETFL, 0);
    if (flags != -1)
    {
        fcntl(app->pipe_fd[0], F_SETFL, flags | O_NONBLOCK);
    }

    app->busy = 1;
    app->current_pid = pid;

    log_info(app, "Запуск команды: %s", cmd);
    return 1;
}

void read_process_output(AppState *app)
{
    if (!app || !app->busy || app->pipe_fd[0] == -1)
    {
        return;
    }

    drain_process_pipe(app, 1);
}

void check_process(AppState *app)
{
    if (!app || !app->busy)
    {
        return;
    }

    read_process_output(app);

    int status = 0;
    pid_t result = waitpid(app->current_pid, &status, WNOHANG);

    if (result == 0)
    {
        return;
    }

    if (result == -1)
    {
        log_error(app, "Ошибка waitpid: %s", strerror(errno));
        state_clear_pending_update(app);
        reset_process_state(app);
        flush_output_tail(app);
        return;
    }

    drain_process_pipe(app, 0);
    flush_output_tail(app);

    close_pipe_end(&app->pipe_fd[0]);
    app->busy = 0;
    app->current_pid = -1;

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);

        if (exit_code == 0)
        {
            state_apply_pending_update(app);
            log_info(app, "Операция завершена успешно.");
        }
        else
        {
            state_clear_pending_update(app);
            log_error(app, "Операция завершилась с кодом %d.", exit_code);
        }
    }
    else if (WIFSIGNALED(status))
    {
        state_clear_pending_update(app);
        log_error(app, "Процесс завершён сигналом %d.", WTERMSIG(status));
    }
    else
    {
        state_clear_pending_update(app);
        log_warn(app, "Процесс завершён с неопределённым статусом.");
    }
}

void terminate_running_process(AppState *app)
{
    if (!app || !app->busy || app->current_pid <= 0)
    {
        return;
    }

    kill(app->current_pid, SIGTERM);
    waitpid(app->current_pid, NULL, 0);

    drain_process_pipe(app, 0);
    flush_output_tail(app);
    state_clear_pending_update(app);
    reset_process_state(app);
}