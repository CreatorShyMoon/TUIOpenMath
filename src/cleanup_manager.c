
#include "cleanup_manager.h"

#include <stdio.h>

int cleanup_build_remove_julia_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "rm -f \"tools/bin/julia\"; "
        "rm -rf \"tools/julia\"; "
        "rm -f \"state/julia.version\"; "
        "echo \"[INFO] Julia removed\""
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int cleanup_build_remove_jupyter_command(char *out_cmd, size_t out_cmd_size)
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
        "  kill \"$PID\" >/dev/null 2>&1 || true; "
        "  rm -f state/jupyter.pid; "
        "fi; "
        "rm -f \"tools/bin/jupyter\"; "
        "rm -f \"tools/bin/jupyter-lab\"; "
        "rm -rf \"tools/jupyter\"; "
        "rm -f \"state/jupyter.version\"; "
        "for _kdir in "
        "  \"$HOME/.local/share/jupyter/kernels\" "
        "  $HOME/snap/code/*/.local/share/jupyter/kernels "
        "  \"/usr/local/share/jupyter/kernels\" "
        "  \"/usr/share/jupyter/kernels\"; do "
        "  if [ -d \"$_kdir\" ]; then "
        "    rm -rf \"${_kdir}\"/sysopenmath-* \"${_kdir}\"/SysOpenMath* "
        "           \"${_kdir}/bash\" \"${_kdir}/octave\" \"${_kdir}/scilab\"; "
        "    echo \"[INFO] Kernels cleared in: $_kdir\"; "
        "  fi; "
        "done; "
        "echo \"[INFO] Jupyter removed\""
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int cleanup_build_clear_kernels_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "FOUND=0; "
        "for dir in "
        "  \"tools/jupyter/runtime/share/jupyter/kernels\" "
        "  \"$HOME/.local/share/jupyter/kernels\" "
        "  $HOME/snap/code/*/.local/share/jupyter/kernels "
        "  \"/usr/local/share/jupyter/kernels\" "
        "  \"/usr/share/jupyter/kernels\"; "
        "do "
        "  if [ -d \"$dir\" ]; then "
        "    FOUND=1; "
        "    rm -rf \"${dir}\"/sysopenmath-* \"${dir}\"/SysOpenMath*; "
        "    rm -rf \"${dir}/bash\" \"${dir}/octave\" \"${dir}/scilab\"; "
        "    rm -rf \"${dir}/python3\"; "
        "    echo \"[INFO] Checked kernels dir: $dir\"; "
        "  fi; "
        "done; "
        "if [ \"$FOUND\" = \"1\" ]; then "
        "  echo \"[INFO] SysOpenMath kernels cleared\"; "
        "else "
        "  echo \"[WARN] no known kernels directories found\"; "
        "fi"
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int cleanup_build_clear_state_command(char *out_cmd, size_t out_cmd_size)
{
    if (!out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "rm -rf \"state\"; "
        "mkdir -p \"state/profiles\"; "
        "echo \"[INFO] State cleared\""
        "'"
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}
