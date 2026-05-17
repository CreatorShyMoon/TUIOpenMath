#include "install_steps.h"
#include "profiles.h"
#include "julia_manager.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int mkstemp(char *template);
FILE *fdopen(int fd, const char *mode);

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

static int write_header(FILE *f)
{
    if (!f)
    {
        return 0;
    }

    return fprintf(
        f,
        "#!/usr/bin/env bash\n"
        "set -euo pipefail\n\n"
        "echo \"[INFO] start\"\n"
        "if ! command -v bash >/dev/null 2>&1; then\n"
        "  echo \"[ERROR] bash не найден\" >&2\n"
        "  exit 127\n"
        "fi\n"
        "if ! command -v python3 >/dev/null 2>&1; then\n"
        "  echo \"[ERROR] python3 не найден\" >&2\n"
        "  exit 127\n"
        "fi\n\n"
    ) > 0;
}

static int write_python_section(FILE *f, const Profile *profile, const ProfileConfig *config, int update_mode)
{
    if (!f || !profile || !config)
    {
        return 0;
    }

    if (fprintf(f, "mkdir -p envs\n") < 0)
    {
        return 0;
    }

    if (update_mode)
    {
        if (fprintf(
                f,
                "if [ ! -d \"envs/%s\" ]; then\n"
                "  echo \"[STEP] create missing python env: %s\"\n"
                "  python3 -m venv \"envs/%s\"\n"
                "fi\n\n",
                profile->name,
                profile->name,
                profile->name
            ) < 0)
        {
            return 0;
        }
    }
    else
    {
        if (fprintf(
                f,
                "echo \"[STEP] create python env: %s\"\n"
                "rm -rf \"envs/%s\"\n"
                "python3 -m venv \"envs/%s\"\n\n",
                profile->name,
                profile->name,
                profile->name
            ) < 0)
        {
            return 0;
        }
    }

    if (config->python.count > 0)
    {
        if (fprintf(
                f,
                "echo \"[STEP] upgrade pip in envs/%s\"\n"
                "\"envs/%s/bin/pip\" install --upgrade pip\n\n",
                profile->name,
                profile->name
            ) < 0)
        {
            return 0;
        }

        if (fprintf(
                f,
                "echo \"[STEP] install python packages for %s\"\n"
                "\"envs/%s/bin/pip\" install",
                profile->name,
                profile->name
            ) < 0)
        {
            return 0;
        }

        if (update_mode)
        {
            if (fprintf(f, " --upgrade") < 0)
            {
                return 0;
            }
        }

        for (size_t i = 0; i < config->python.count; i++)
        {
            if (fprintf(f, " %s", config->python.items[i]) < 0)
            {
                return 0;
            }
        }

        if (fprintf(f, "\n\n") < 0)
        {
            return 0;
        }
    }

    return 1;
}

static int write_system_item(FILE *f, const char *item)
{
    if (!f || !item)
    {
        return 0;
    }

    if (strcmp(item, "octave") == 0)
    {
        return fprintf(
            f,
            "if command -v octave >/dev/null 2>&1; then\n"
            "  echo \"[INFO] octave already installed: $(octave --version | head -n 1)\"\n"
            "else\n"
            "  echo \"[STEP] install octave\"\n"
            "  sudo apt-get install -y octave\n"
            "fi\n\n"
        ) > 0;
    }

    if (strcmp(item, "scilab") == 0)
    {
        return fprintf(
            f,
            "if command -v scilab-adv-cli >/dev/null 2>&1 || command -v scilab-cli >/dev/null 2>&1 || command -v scilab >/dev/null 2>&1; then\n"
            "  echo \"[INFO] scilab already installed\"\n"
            "else\n"
            "  echo \"[STEP] install scilab\"\n"
            "  sudo apt-get install -y scilab\n"
            "fi\n\n"
        ) > 0;
    }

    return fprintf(
        f,
        "if dpkg -s \"%s\" >/dev/null 2>&1; then\n"
        "  echo \"[INFO] package already installed: %s\"\n"
        "else\n"
        "  echo \"[STEP] install system package: %s\"\n"
        "  sudo apt-get install -y \"%s\"\n"
        "fi\n\n",
        item, item, item, item
    ) > 0;
}

static int write_system_section(FILE *f, const ProfileConfig *config)
{
    if (!f || !config)
    {
        return 0;
    }

    if (config->system.count == 0)
    {
        return 1;
    }

    if (fprintf(
            f,
            "if ! command -v sudo >/dev/null 2>&1; then\n"
            "  echo \"[ERROR] sudo не найден\" >&2\n"
            "  exit 127\n"
            "fi\n"
            "if ! command -v apt-get >/dev/null 2>&1; then\n"
            "  echo \"[ERROR] apt-get не найден\" >&2\n"
            "  exit 127\n"
            "fi\n\n"
            "echo \"[STEP] sync apt package index\"\n"
            "sudo apt-get update\n\n"
        ) < 0)
    {
        return 0;
    }

    for (size_t i = 0; i < config->system.count; i++)
    {
        if (!write_system_item(f, config->system.items[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int write_julia_section(FILE *f, const Profile *profile, const ProfileConfig *config, int update_mode)
{
    if (!f || !profile || !config)
    {
        return 0;
    }

    if (config->julia.count == 0)
    {
        return 1;
    }

    char julia_bin[2048];
    if (!julia_get_binary_path(julia_bin, sizeof(julia_bin)))
    {
        return 0;
    }

    if (fprintf(
            f,
            "if ! [ -x \"%s\" ]; then\n"
            "  echo \"[ERROR] Julia не установлена. Сначала выбери 'Установить Julia LTS' или 'Установить Julia Stable'.\" >&2\n"
            "  exit 1\n"
            "fi\n\n",
            julia_bin
        ) < 0)
    {
        return 0;
    }

    if (fprintf(f, "echo \"[STEP] install julia packages for %s\"\n", profile->name) < 0)
    {
        return 0;
    }

    for (size_t i = 0; i < config->julia.count; i++)
    {
        const char *pkg = config->julia.items[i];

        if (update_mode)
        {
            if (fprintf(
                    f,
                    "\"%s\" -e 'using Pkg; Pkg.add(\"%s\"); Pkg.update(\"%s\")'\n",
                    julia_bin,
                    pkg,
                    pkg
                ) < 0)
            {
                return 0;
            }
        }
        else
        {
            if (fprintf(
                    f,
                    "\"%s\" -e 'using Pkg; Pkg.add(\"%s\")'\n",
                    julia_bin,
                    pkg
                ) < 0)
            {
                return 0;
            }
        }
    }

    if (fprintf(f, "\n") < 0)
    {
        return 0;
    }

    return 1;
}

static int build_temp_script_path(char *buffer, size_t buffer_size, const char *profile_name, int update_mode)
{
    if (!buffer || buffer_size == 0 || !profile_name)
    {
        return 0;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "/tmp/sysopenmath_%s_%s_XXXXXX",
        profile_name,
        update_mode ? "update" : "install"
    );

    return written >= 0 && (size_t)written < buffer_size;
}

int build_install_command(
    const Profile *profile,
    const ProfileConfig *config,
    int update_mode,
    char *out_cmd,
    size_t out_cmd_size
)
{
    if (!profile || !config || !out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    if (!profile_name_is_safe(profile->name))
    {
        return 0;
    }

    if (!ensure_dir_exists("/tmp"))
    {
        return 0;
    }

    char script_template[512];
    if (!build_temp_script_path(script_template, sizeof(script_template), profile->name, update_mode))
    {
        return 0;
    }

    int fd = mkstemp(script_template);
    if (fd == -1)
    {
        return 0;
    }

    FILE *f = fdopen(fd, "wb");
    if (!f)
    {
        close(fd);
        unlink(script_template);
        return 0;
    }

    int ok = write_header(f) &&
             write_python_section(f, profile, config, update_mode) &&
             write_system_section(f, config) &&
             write_julia_section(f, profile, config, update_mode) &&
             fprintf(f, "echo \"[INFO] done\"\n") > 0;

    if (!ok)
    {
        fclose(f);
        unlink(script_template);
        return 0;
    }

    if (fclose(f) != 0)
    {
        unlink(script_template);
        return 0;
    }

    if (chmod(script_template, 0755) != 0)
    {
        unlink(script_template);
        return 0;
    }

    int written = snprintf(out_cmd, out_cmd_size, "bash \"%s\"", script_template);
    if (written < 0 || (size_t)written >= out_cmd_size)
    {
        unlink(script_template);
        return 0;
    }

    return 1;
}

int build_delete_python_env_command(
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

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "if [ -d \"envs/%s\" ]; then "
        "  rm -rf \"envs/%s\"; "
        "  echo \"[INFO] deleted envs/%s\"; "
        "else "
        "  echo \"[WARN] envs/%s not found\"; "
        "fi"
        "'",
        profile->name,
        profile->name,
        profile->name,
        profile->name
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}
