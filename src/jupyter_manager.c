#include "jupyter_manager.h"

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define JUPYTER_RUNTIME_DIR "tools/jupyter/runtime"
#define JUPYTER_BIN_DIR     "tools/bin"
#define JUPYTER_BIN_PATH    "tools/bin/jupyter"
#define JUPYTER_LAB_PATH    "tools/bin/jupyter-lab"

int lstat(const char *path, struct stat *buf);

int jupyter_get_binary_path(char *out, size_t out_size)
{
    if (!out || out_size == 0)
    {
        return 0;
    }

    int written = snprintf(out, out_size, "%s", JUPYTER_BIN_PATH);
    return written >= 0 && (size_t)written < out_size;
}

int jupyter_get_lab_binary_path(char *out, size_t out_size)
{
    if (!out || out_size == 0)
    {
        return 0;
    }

    int written = snprintf(out, out_size, "%s", JUPYTER_LAB_PATH);
    return written >= 0 && (size_t)written < out_size;
}

int jupyter_is_link_present(void)
{
    struct stat st;
    return lstat(JUPYTER_BIN_PATH, &st) == 0;
}

int jupyter_is_installed(void)
{
    return access(JUPYTER_BIN_PATH, X_OK) == 0;
}

int jupyter_is_runtime_healthy(void)
{
    struct stat st;
    if (lstat(JUPYTER_BIN_PATH, &st) != 0)
    {
        return 0;
    }

    if (access(JUPYTER_BIN_PATH, X_OK) != 0)
    {
        return 0;
    }

    if (access(JUPYTER_LAB_PATH, X_OK) != 0)
    {
        return 0;
    }

    return 1;
}

int jupyter_build_install_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "set -euo pipefail; "
        "if ! command -v python3 >/dev/null 2>&1; then echo \"[ERROR] python3 не найден\" >&2; exit 127; fi; "
        "if [ -x \"%s\" ] && [ -x \"%s\" ]; then "
        "  echo \"[INFO] Jupyter already installed\"; "
        "  \"%s\" --version; "
        "  exit 0; "
        "fi; "
        "mkdir -p \"%s\" \"%s\"; "
        "echo \"[STEP] create Jupyter runtime\"; "
        "rm -rf \"%s\"; "
        "python3 -m venv \"%s\"; "
        "echo \"[STEP] upgrade pip in Jupyter runtime\"; "
        "\"%s/bin/pip\" install --upgrade pip; "
        "echo \"[STEP] install Jupyter packages\"; "
        "\"%s/bin/pip\" install jupyterlab notebook ipykernel bash_kernel octave_kernel scilab_kernel; "
        "ln -sfn \"$(pwd)/%s/bin/jupyter\" \"%s\"; "
        "ln -sfn \"$(pwd)/%s/bin/jupyter-lab\" \"%s\"; "
        "echo \"[INFO] Jupyter installed\"; "
        "\"%s\" --version"
        "'",
        JUPYTER_BIN_PATH,
        JUPYTER_LAB_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_BIN_DIR,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_BIN_PATH,
        JUPYTER_RUNTIME_DIR,
        JUPYTER_LAB_PATH,
        JUPYTER_BIN_PATH
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int jupyter_build_version_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "if [ -L \"%s\" ] && [ ! -e \"%s\" ]; then "
        "  echo \"[FAIL] Jupyter symlink is broken: %s\"; "
        "elif [ -x \"%s\" ]; then "
        "  \"%s\" --version; "
        "else "
        "  echo \"Jupyter not installed\"; "
        "fi"
        "'",
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int jupyter_build_kernels_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "if [ -L \"%s\" ] && [ ! -e \"%s\" ]; then "
        "  echo \"[FAIL] Jupyter symlink is broken: %s\"; "
        "elif [ -x \"%s\" ]; then "
        "  \"%s\" kernelspec list; "
        "else "
        "  echo \"Jupyter not installed\"; "
        "fi"
        "'",
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH,
        JUPYTER_BIN_PATH
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int jupyter_build_lab_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "set -euo pipefail; "
        "LOG_FILE=\"/tmp/sysopenmath_jupyterlab.log\"; "
        "if [ -L \"%s\" ] && [ ! -e \"%s\" ]; then "
        "  echo \"[FAIL] JupyterLab symlink is broken: %s\"; "
        "elif [ -x \"%s\" ]; then "
        "  rm -f \"$LOG_FILE\"; "
        "  nohup \"%s\" --no-browser > \"$LOG_FILE\" 2>&1 < /dev/null & "
        "  JPID=$!; mkdir -p state; echo \"$JPID\" > state/jupyter.pid; "
        "  echo \"[INFO] JupyterLab started in background (pid=$JPID)\"; "
        "  sleep 3; "
        "  URL=$(grep -Eo \"http://(127\\.0\\.0\\.1|localhost):[0-9]+(/lab)?\\?token=[^[:space:]]+\" \"$LOG_FILE\" | head -n 1 || true); "
        "  if [ -z \"$URL\" ]; then "
        "    URL=$(grep -Eo \"http://(127\\.0\\.0\\.1|localhost):[0-9]+/[^[:space:]]*token=[^[:space:]]+\" \"$LOG_FILE\" | head -n 1 || true); "
        "  fi; "
        "  if [ -n \"$URL\" ]; then "
        "    echo \"[INFO] Access URL: $URL\"; "
        "  else "
        "    echo \"[WARN] JupyterLab URL not found yet. Check /tmp/sysopenmath_jupyterlab.log\"; "
        "  fi; "
        "else "
        "  echo \"JupyterLab not installed\"; "
        "fi"
        "'",
        JUPYTER_LAB_PATH,
        JUPYTER_LAB_PATH,
        JUPYTER_LAB_PATH,
        JUPYTER_LAB_PATH,
        JUPYTER_LAB_PATH
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int jupyter_build_remove_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "set -euo pipefail; "
        "if [ -f state/jupyter.pid ]; then "
        "  JPID=$(cat state/jupyter.pid); "
        "  kill \"$JPID\" >/dev/null 2>&1 || true; "
        "  rm -f state/jupyter.pid; "
        "fi; "
        "pkill -f jupyter-lab >/dev/null 2>&1 || true; "
        "rm -f \"%s\" \"%s\"; "
        "rm -rf \"%s\"; "
        "echo \"[INFO] Jupyter removed\""
        "'",
        JUPYTER_BIN_PATH,
        JUPYTER_LAB_PATH,
        JUPYTER_RUNTIME_DIR
    );
    
    return written >= 0 && (size_t)written < out_cmd_size;
}