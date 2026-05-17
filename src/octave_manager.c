#include "octave_manager.h"

#include <stdio.h>

int octave_build_install_command(char *out_cmd, size_t out_cmd_size)
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
        "echo \"[STEP] detect octave\"; "
        "if command -v octave >/dev/null 2>&1; then "
        "  echo \"[INFO] octave already installed: $(octave --version | head -n 1)\"; "
        "  echo \"[INFO] installation skipped\"; "
        "else "
        "  echo \"[STEP] install octave\"; "
        "  sudo apt-get update; "
        "  sudo apt-get install -y octave; "
        "  octave --version | head -n 1; "
        "fi"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int octave_build_remove_command(char *out_cmd, size_t out_cmd_size)
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
        "echo \"[STEP] remove octave\"; "
        "if command -v octave >/dev/null 2>&1; then "
        "  sudo apt-get remove -y octave; "
        "  sudo apt-get autoremove -y; "
        "  echo \"[INFO] octave removed\"; "
        "else "
        "  echo \"[WARN] octave is not installed\"; "
        "fi; "
        "command -v octave >/dev/null 2>&1 && octave --version | head -n 1 || echo \"[INFO] octave command not found\""
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}
