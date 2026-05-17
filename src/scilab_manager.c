#include "scilab_manager.h"

#include <stdio.h>

int scilab_build_install_command(char *out_cmd, size_t out_cmd_size)
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
        "echo \"[STEP] detect scilab\"; "
        "if command -v scilab-adv-cli >/dev/null 2>&1; then "
        "  echo \"[INFO] scilab already installed: $(scilab-adv-cli -version | head -n 1)\"; "
        "  echo \"[INFO] installation skipped\"; "
        "elif command -v scilab-cli >/dev/null 2>&1; then "
        "  echo \"[INFO] scilab already installed: $(scilab-cli -version | head -n 1)\"; "
        "  echo \"[INFO] installation skipped\"; "
        "elif command -v scilab >/dev/null 2>&1; then "
        "  echo \"[INFO] scilab already installed: $(scilab -version | head -n 1)\"; "
        "  echo \"[INFO] installation skipped\"; "
        "else "
        "  echo \"[STEP] install scilab\"; "
        "  sudo apt-get update; "
        "  sudo apt-get install -y scilab; "
        "  if command -v scilab-adv-cli >/dev/null 2>&1; then scilab-adv-cli -version | head -n 1; "
        "  elif command -v scilab-cli >/dev/null 2>&1; then scilab-cli -version | head -n 1; "
        "  elif command -v scilab >/dev/null 2>&1; then scilab -version | head -n 1; "
        "  else echo \"[ERROR] Scilab executable not found after install\"; exit 1; fi; "
        "fi"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int scilab_build_remove_command(char *out_cmd, size_t out_cmd_size)
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
        "echo \"[STEP] remove scilab\"; "
        "if command -v scilab >/dev/null 2>&1 || command -v scilab-cli >/dev/null 2>&1 || command -v scilab-adv-cli >/dev/null 2>&1; then "
        "  sudo apt-get remove -y scilab; "
        "  sudo apt-get autoremove -y; "
        "  echo \"[INFO] scilab removed\"; "
        "else "
        "  echo \"[WARN] scilab is not installed\"; "
        "fi; "
        "if command -v scilab-adv-cli >/dev/null 2>&1 || command -v scilab-cli >/dev/null 2>&1 || command -v scilab >/dev/null 2>&1; then "
        "  echo \"[WARN] Scilab executable still present\"; "
        "else "
        "  echo \"[INFO] scilab command not found\"; "
        "fi"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int scilab_build_register_kernel_command(char *out_cmd, size_t out_cmd_size)
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
        "PYBIN=\"tools/jupyter/runtime/bin/python\"; "
        "if [ ! -x \"$PYBIN\" ]; then "
        "  echo \"[ERROR] Jupyter runtime not installed\"; "
        "  exit 1; "
        "fi; "
        "if command -v scilab-adv-cli >/dev/null 2>&1; then "
        "  export SCILAB_EXECUTABLE=\"$(command -v scilab-adv-cli)\"; "
        "elif command -v scilab-cli >/dev/null 2>&1; then "
        "  export SCILAB_EXECUTABLE=\"$(command -v scilab-cli)\"; "
        "elif command -v scilab >/dev/null 2>&1; then "
        "  export SCILAB_EXECUTABLE=\"$(command -v scilab)\"; "
        "else "
        "  echo \"[ERROR] Scilab executable not found\"; "
        "  exit 1; "
        "fi; "
        "\"$PYBIN\" -m pip install scilab_kernel; "
        "\"$PYBIN\" -m scilab_kernel install --user; "
        "echo \"[INFO] Scilab kernel registered\"; "
        "tools/bin/jupyter kernelspec list"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}
