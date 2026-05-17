#include "setup_manager.h"
#include "profile_config.h"
#include "profiles.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static int appendf(char *out, size_t out_size, size_t *offset, const char *fmt, ...)
{
    if (!out || !offset || !fmt || *offset >= out_size)
    {
        return 0;
    }

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(out + *offset, out_size - *offset, fmt, args);
    va_end(args);

    if (written < 0 || (size_t)written >= out_size - *offset)
    {
        return 0;
    }

    *offset += (size_t)written;
    return 1;
}

static int list_contains(const StringList *list, const char *value)
{
    if (!list || !value)
    {
        return 0;
    }

    for (size_t i = 0; i < list->count; i++)
    {
        if (strcmp(list->items[i], value) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int setup_build_full_profile_command(
    const Profile *profile,
    char *out_cmd,
    size_t out_cmd_size
)
{
    if (!profile || !out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    if (!profile_name_is_safe(profile->name))
    {
        return 0;
    }

    ProfileConfig config;
    if (!profile_config_load(profile->name, &config))
    {
        return 0;
    }

    size_t off = 0;
    int ok = appendf(out_cmd, out_cmd_size, &off, "bash -lc 'set -euo pipefail; PROFILE=\"%s\"; LOG_PREFIX=\"[SETUP]\"; ", profile->name);

    ok = ok && appendf(out_cmd, out_cmd_size, &off,
        "if [ -x \"tools/bin/julia\" ]; then echo \"$LOG_PREFIX Julia already installed: $(tools/bin/julia --version)\"; "
        "else "
        "echo \"$LOG_PREFIX install Julia LTS\"; "
        "mkdir -p tools/julia tools/bin; "
        "ARCHIVE=\"julia-1.10.10-linux-x86_64.tar.gz\"; "
        "URL=\"https://julialang-s3.julialang.org/bin/linux/x64/1.10/$ARCHIVE\"; "
        "TARGET_DIR=\"tools/julia/lts\"; "
        "TMP_ARCHIVE=$(mktemp /tmp/sysopenmath_julia_XXXXXX.tar.gz); "
        "rm -rf \"$TARGET_DIR\"; mkdir -p \"$TARGET_DIR\"; "
        "curl -fL \"$URL\" -o \"$TMP_ARCHIVE\"; "
        "tar -xzf \"$TMP_ARCHIVE\" -C \"$TARGET_DIR\" --strip-components=1; "
        "rm -f \"$TMP_ARCHIVE\"; "
        "ln -sfn \"$(pwd)/$TARGET_DIR/bin/julia\" tools/bin/julia; "
        "tools/bin/julia --version; "
        "fi; ");

    ok = ok && appendf(out_cmd, out_cmd_size, &off,
        "if [ -x \"tools/bin/jupyter\" ] && [ -x \"tools/bin/jupyter-lab\" ]; then "
        "echo \"$LOG_PREFIX Jupyter already installed\"; tools/bin/jupyter --version; "
        "else "
        "echo \"$LOG_PREFIX install Jupyter runtime\"; "
        "mkdir -p tools/jupyter tools/bin; rm -rf tools/jupyter/runtime; "
        "python3 -m venv tools/jupyter/runtime; "
        "tools/jupyter/runtime/bin/pip install --upgrade pip; "
        "tools/jupyter/runtime/bin/pip install jupyterlab notebook ipykernel bash_kernel octave_kernel scilab_kernel; "
        "ln -sfn \"$(pwd)/tools/jupyter/runtime/bin/jupyter\" tools/bin/jupyter; "
        "ln -sfn \"$(pwd)/tools/jupyter/runtime/bin/jupyter-lab\" tools/bin/jupyter-lab; "
        "tools/bin/jupyter --version; "
        "fi; ");

    ok = ok && appendf(out_cmd, out_cmd_size, &off,
        "echo \"$LOG_PREFIX install profile $PROFILE\"; mkdir -p envs; rm -rf \"envs/$PROFILE\"; python3 -m venv \"envs/$PROFILE\"; "
        "\"envs/$PROFILE/bin/pip\" install --upgrade pip; ");

    if (config.python.count > 0)
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off, "\"envs/$PROFILE/bin/pip\" install");
        for (size_t i = 0; i < config.python.count; i++)
        {
            ok = ok && appendf(out_cmd, out_cmd_size, &off, " %s", config.python.items[i]);
        }
        ok = ok && appendf(out_cmd, out_cmd_size, &off, "; ");
    }

    for (size_t i = 0; i < config.system.count; i++)
    {
        const char *item = config.system.items[i];
        if (strcmp(item, "octave") == 0)
        {
            ok = ok && appendf(out_cmd, out_cmd_size, &off,
                "if command -v octave >/dev/null 2>&1; then echo \"[INFO] octave already installed: $(octave --version | head -n 1)\"; "
                "else sudo apt-get update; sudo apt-get install -y octave; fi; ");
        }
        else if (strcmp(item, "scilab") == 0)
        {
            ok = ok && appendf(out_cmd, out_cmd_size, &off,
                "if command -v scilab-adv-cli >/dev/null 2>&1 || command -v scilab-cli >/dev/null 2>&1 || command -v scilab >/dev/null 2>&1; then "
                "echo \"[INFO] scilab already installed\"; else sudo apt-get update; sudo apt-get install -y scilab; fi; ");
        }
        else
        {
            ok = ok && appendf(out_cmd, out_cmd_size, &off,
                "if dpkg -s \"%s\" >/dev/null 2>&1; then echo \"[INFO] package already installed: %s\"; "
                "else sudo apt-get update; sudo apt-get install -y \"%s\"; fi; ",
                item, item, item);
        }
    }

    for (size_t i = 0; i < config.julia.count; i++)
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off,
            "tools/bin/julia -e \"using Pkg; Pkg.add(\\\"%s\\\")\"; ",
            config.julia.items[i]);
    }

    if (list_contains(&config.kernels, "python"))
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off,
            "\"envs/$PROFILE/bin/python\" -m ipykernel install --user --name \"sysopenmath-$PROFILE\" --display-name \"SysOpenMath Python ($PROFILE)\"; ");
    }

    if (list_contains(&config.kernels, "julia"))
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off,
            "tools/bin/julia -e \"using Pkg; Pkg.add(\\\"IJulia\\\"); using IJulia; installkernel(\\\"SysOpenMath Julia\\\")\"; ");
    }

    if (list_contains(&config.kernels, "bash"))
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off,
            "tools/jupyter/runtime/bin/python -m bash_kernel.install --user; ");
    }

    if (list_contains(&config.kernels, "octave"))
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off,
            "if command -v octave >/dev/null 2>&1; then tools/jupyter/runtime/bin/python -m octave_kernel install --user; else echo \"[WARN] octave command not found, octave kernel skipped\"; fi; ");
    }

    if (list_contains(&config.kernels, "scilab"))
    {
        ok = ok && appendf(out_cmd, out_cmd_size, &off,
            "if command -v scilab-adv-cli >/dev/null 2>&1; then export SCILAB_EXECUTABLE=\"$(command -v scilab-adv-cli)\"; "
            "elif command -v scilab-cli >/dev/null 2>&1; then export SCILAB_EXECUTABLE=\"$(command -v scilab-cli)\"; "
            "elif command -v scilab >/dev/null 2>&1; then export SCILAB_EXECUTABLE=\"$(command -v scilab)\"; fi; "
            "if [ -n \"${SCILAB_EXECUTABLE:-}\" ]; then tools/jupyter/runtime/bin/python -m scilab_kernel install --user; else echo \"[WARN] scilab command not found, scilab kernel skipped\"; fi; ");
    }

    ok = ok && appendf(out_cmd, out_cmd_size, &off,
        "echo \"$LOG_PREFIX create workspace\"; "
        "WS_DIR=\"workspaces/$PROFILE\"; mkdir -p \"$WS_DIR\"; NOTEBOOK=\"$WS_DIR/${PROFILE}_starter.ipynb\"; README=\"$WS_DIR/README.md\"; "
        "cat > \"$README\" <<EOF\n"
        "# SysOpenMath Workspace: $PROFILE\n\n"
        "Готовый workspace для профиля $PROFILE.\n\n"
        "## Что дальше\n"
        "1. Запусти JupyterLab\n"
        "2. Открой notebook ${PROFILE}_starter.ipynb\n"
        "3. Проверь kernels и состояние системы\n"
        "EOF\n"
        "if [ ! -f \"$NOTEBOOK\" ]; then "
        "cat > \"$NOTEBOOK\" <<EOF\n"
        "{\n"
        "  \"cells\": [],\n"
        "  \"metadata\": {\n"
        "    \"kernelspec\": {\n"
        "      \"display_name\": \"SysOpenMath Python ($PROFILE)\",\n"
        "      \"language\": \"python\",\n"
        "      \"name\": \"sysopenmath-$PROFILE\"\n"
        "    },\n"
        "    \"language_info\": {\n"
        "      \"name\": \"python\"\n"
        "    }\n"
        "  },\n"
        "  \"nbformat\": 4,\n"
        "  \"nbformat_minor\": 5\n"
        "}\n"
        "EOF\n"
        "fi; "
        "echo \"$LOG_PREFIX update state\"; mkdir -p state/profiles; "
        "tools/bin/julia --version > state/julia.version || true; "
        "tools/bin/jupyter --version > state/jupyter.version || true; "
        "printf 'profile=%%s\\npython_env=envs/%%s\\n' \"$PROFILE\" \"$PROFILE\" > \"state/profiles/$PROFILE.state\"; "
        "echo \"[INFO] full setup completed for profile: $PROFILE\"'");

    profile_config_free(&config);
    return ok;
}
