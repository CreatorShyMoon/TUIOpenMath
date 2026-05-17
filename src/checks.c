#include "checks.h"
#include "logger.h"
#include "process.h"
#include "profile_config.h"
#include "profiles.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int mkstemp(char *template);
FILE *fdopen(int fd, const char *mode);

static int string_list_contains_value(const StringList *list, const char *value)
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

static int write_base_checks(FILE *f)
{
    if (!f)
    {
        return 0;
    }

    return fprintf(
        f,
        "#!/usr/bin/env bash\n"
        "set -euo pipefail\n\n"
        "echo \"[CHECK] system\"\n"
        "command -v python3 >/dev/null 2>&1 && echo \"[OK] python3\" || echo \"[FAIL] python3\"\n"
        "command -v sudo >/dev/null 2>&1 && echo \"[OK] sudo\" || echo \"[FAIL] sudo\"\n"
        "command -v apt-get >/dev/null 2>&1 && echo \"[OK] apt-get\" || echo \"[FAIL] apt-get\"\n"
        "\n"
        "echo \"[CHECK] jupyter platform\"\n"
        "if [ -L \"tools/bin/jupyter\" ] && [ ! -e \"tools/bin/jupyter\" ]; then\n"
        "  echo \"[FAIL] tools/bin/jupyter broken symlink\"\n"
        "elif [ -x \"tools/bin/jupyter\" ]; then\n"
        "  echo \"[OK] tools/bin/jupyter\"\n"
        "  tools/bin/jupyter --version\n"
        "else\n"
        "  echo \"[FAIL] tools/bin/jupyter\"\n"
        "fi\n"
        "\n"
        "echo \"[CHECK] julia platform\"\n"
        "if [ -L \"tools/bin/julia\" ] && [ ! -e \"tools/bin/julia\" ]; then\n"
        "  echo \"[FAIL] tools/bin/julia broken symlink\"\n"
        "elif [ -x \"tools/bin/julia\" ]; then\n"
        "  echo \"[OK] tools/bin/julia\"\n"
        "  tools/bin/julia --version\n"
        "else\n"
        "  echo \"[FAIL] tools/bin/julia\"\n"
        "fi\n\n"
    ) > 0;
}

static int write_system_dependency_checks(FILE *f, const Profile *profile, const ProfileConfig *config)
{
    if (!f || !profile || !config)
    {
        return 0;
    }

    if (fprintf(f, "echo \"[CHECK] system dependencies for %s\"\n", profile->name) < 0)
    {
        return 0;
    }

    if (config->system.count == 0)
    {
        return fprintf(f, "echo \"[SKIP] no system dependencies required\"\n\n") >= 0;
    }

    for (size_t i = 0; i < config->system.count; i++)
    {
        const char *pkg = config->system.items[i];

        if (strcmp(pkg, "octave") == 0)
        {
            if (fprintf(
                    f,
                    "command -v octave >/dev/null 2>&1 && echo \"[OK] octave command\" || echo \"[FAIL] octave command\"\n"
                ) < 0)
            {
                return 0;
            }
        }
        else if (strcmp(pkg, "scilab") == 0)
        {
            if (fprintf(
                    f,
                    "(command -v scilab-adv-cli >/dev/null 2>&1 || command -v scilab-cli >/dev/null 2>&1 || command -v scilab >/dev/null 2>&1) && echo \"[OK] scilab command\" || echo \"[FAIL] scilab command\"\n"
                ) < 0)
            {
                return 0;
            }
        }
        else
        {
            if (fprintf(
                    f,
                    "command -v %s >/dev/null 2>&1 && echo \"[OK] %s command\" || echo \"[FAIL] %s command\"\n",
                    pkg,
                    pkg,
                    pkg
                ) < 0)
            {
                return 0;
            }
        }
    }

    return fprintf(f, "\n") >= 0;
}

static int write_kernel_checks(FILE *f, const Profile *profile, const ProfileConfig *config)
{
    if (!f || !profile || !config)
    {
        return 0;
    }

    if (fprintf(f, "echo \"[CHECK] kernels for %s\"\n", profile->name) < 0)
    {
        return 0;
    }

    if (config->kernels.count == 0)
    {
        return fprintf(f, "echo \"[SKIP] no kernels required\"\n\n") >= 0;
    }

    if (fprintf(f, "if [ -x \"tools/bin/jupyter\" ]; then\n") < 0)
    {
        return 0;
    }

    if (fprintf(f, "  KS=$(tools/bin/jupyter kernelspec list 2>/dev/null || true)\n") < 0)
    {
        return 0;
    }

    if (string_list_contains_value(&config->kernels, "python"))
    {
        if (fprintf(
                f,
                "  printf \"%%s\\n\" \"$KS\" | grep -q \"sysopenmath-%s\" && echo \"[OK] python kernel\" || echo \"[FAIL] python kernel\"\n",
                profile->name
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->kernels, "julia"))
    {
        if (fprintf(
                f,
                "  printf \"%%s\\n\" \"$KS\" | grep -Eqi \"sysopenmath-julia\" && echo \"[OK] julia kernel\" || echo \"[FAIL] julia kernel\"\n"
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->kernels, "bash"))
    {
        if (fprintf(
                f,
                "  printf \"%%s\\n\" \"$KS\" | grep -Eqi '(^|[[:space:]])bash([[:space:]]|$)' && echo \"[OK] bash kernel\" || echo \"[FAIL] bash kernel\"\n"
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->kernels, "octave"))
    {
        if (fprintf(
                f,
                "  printf \"%%s\\n\" \"$KS\" | grep -Eqi '(^|[[:space:]])octave([[:space:]]|$)' && echo \"[OK] octave kernel\" || echo \"[FAIL] octave kernel\"\n"
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->kernels, "scilab"))
    {
        if (fprintf(
                f,
                "  printf \"%%s\\n\" \"$KS\" | grep -Eqi '(^|[[:space:]])scilab([[:space:]]|$)' && echo \"[OK] scilab kernel\" || echo \"[FAIL] scilab kernel\"\n"
            ) < 0)
        {
            return 0;
        }
    }

    if (fprintf(
            f,
            "else\n"
            "  echo \"[FAIL] jupyter missing for profile kernels\"\n"
            "fi\n\n"
        ) < 0)
    {
        return 0;
    }

    return 1;
}

static int write_profile_checks(FILE *f, const Profile *profile, const ProfileConfig *config)
{
    if (!f || !profile || !config)
    {
        return 0;
    }

    if (fprintf(
            f,
            "echo \"[CHECK] profile %s\"\n"
            "if [ -x \"envs/%s/bin/python\" ]; then\n"
            "  echo \"[OK] envs/%s/bin/python\"\n"
            "  \"envs/%s/bin/python\" --version\n"
            "else\n"
            "  echo \"[FAIL] envs/%s/bin/python\"\n"
            "fi\n"
            "\n"
            "if [ -x \"envs/%s/bin/pip\" ]; then\n"
            "  echo \"[OK] envs/%s/bin/pip\"\n"
            "else\n"
            "  echo \"[FAIL] envs/%s/bin/pip\"\n"
            "fi\n\n",
            profile->name,
            profile->name,
            profile->name,
            profile->name,
            profile->name,
            profile->name,
            profile->name,
            profile->name
        ) < 0)
    {
        return 0;
    }

    if (!write_system_dependency_checks(f, profile, config))
    {
        return 0;
    }

    if (!write_kernel_checks(f, profile, config))
    {
        return 0;
    }

    if (fprintf(f, "echo \"[CHECK] python imports for %s\"\n", profile->name) < 0)
    {
        return 0;
    }

    if (config->python.count == 0)
    {
        if (fprintf(f, "echo \"[SKIP] no python imports required\"\n") < 0)
        {
            return 0;
        }
    }
    else
    {
        for (size_t i = 0; i < config->python.count; i++)
        {
            const char *pkg = config->python.items[i];

            if (fprintf(
                    f,
                    "\"envs/%s/bin/python\" -c \"import %s; print('[OK] python import: %s')\" || echo \"[FAIL] python import: %s\"\n",
                    profile->name,
                    pkg,
                    pkg,
                    pkg
                ) < 0)
            {
                return 0;
            }
        }
    }

    if (fprintf(
            f,
            "\necho \"[CHECK] profile readiness for %s\"\n"
            "READY=1\n"
            "[ -x \"envs/%s/bin/python\" ] || READY=0\n"
            "[ -x \"envs/%s/bin/pip\" ] || READY=0\n"
            "[ -d \"workspaces/%s\" ] || READY=0\n",
            profile->name,
            profile->name,
            profile->name,
            profile->name
        ) < 0)
    {
        return 0;
    }

    if (string_list_contains_value(&config->kernels, "python"))
    {
        if (fprintf(
                f,
                "if [ -x \"tools/bin/jupyter\" ]; then KS=$(tools/bin/jupyter kernelspec list 2>/dev/null || true); printf \"%%s\\n\" \"$KS\" | grep -q \"sysopenmath-%s\" || READY=0; else READY=0; fi\n",
                profile->name
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->kernels, "julia"))
    {
        if (fprintf(
                f,
                "if [ -x \"tools/bin/jupyter\" ]; then KS=$(tools/bin/jupyter kernelspec list 2>/dev/null || true); printf \"%%s\\n\" \"$KS\" | grep -Eqi \"sysopenmath-julia\" || READY=0; else READY=0; fi\n"
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->kernels, "bash"))
    {
        if (fprintf(
                f,
                "if [ -x \"tools/bin/jupyter\" ]; then KS=$(tools/bin/jupyter kernelspec list 2>/dev/null || true); printf \"%%s\\n\" \"$KS\" | grep -Eqi '(^|[[:space:]])bash([[:space:]]|$)' || READY=0; else READY=0; fi\n"
            ) < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->system, "octave"))
    {
        if (fprintf(f, "command -v octave >/dev/null 2>&1 || READY=0\n") < 0)
        {
            return 0;
        }
    }

    if (string_list_contains_value(&config->system, "scilab"))
    {
        if (fprintf(f, "(command -v scilab-adv-cli >/dev/null 2>&1 || command -v scilab-cli >/dev/null 2>&1 || command -v scilab >/dev/null 2>&1) || READY=0\n") < 0)
        {
            return 0;
        }
    }

    if (fprintf(
            f,
            "if [ \"$READY\" -eq 1 ]; then echo \"[OK] profile readiness: READY\"; else echo \"[WARN] profile readiness: PARTIALLY READY\"; fi\n\n"
        ) < 0)
    {
        return 0;
    }

    if (fprintf(f, "echo \"[CHECK] julia imports for %s\"\n", profile->name) < 0)
    {
        return 0;
    }

    if (config->julia.count == 0)
    {
        if (fprintf(f, "echo \"[SKIP] no julia imports required\"\n") < 0)
        {
            return 0;
        }
    }
    else
    {
        if (fprintf(f, "if [ -x \"tools/bin/julia\" ]; then\n") < 0)
        {
            return 0;
        }

        if (fprintf(f, "  tools/bin/julia -e \"") < 0)
        {
            return 0;
        }

        for (size_t i = 0; i < config->julia.count; i++)
        {
            if (fprintf(f, "using %s; ", config->julia.items[i]) < 0)
            {
                return 0;
            }
        }

        if (fprintf(
                f,
                "println(\\\"[OK] julia imports\\\")\" || echo \"[FAIL] julia imports\"\n"
                "else\n"
                "  echo \"[FAIL] tools/bin/julia\"\n"
                "fi\n"
            ) < 0)
        {
            return 0;
        }
    }

    return 1;
}

void run_component_checks(AppState *app)
{
    if (!app)
    {
        return;
    }

    const Profile *profile = get_selected_profile(app);

    char script_template[512];
    int sw = snprintf(script_template, sizeof(script_template), "/tmp/sysopenmath_checks_XXXXXX");
    if (sw < 0 || (size_t)sw >= sizeof(script_template))
    {
        log_error(app, "Не удалось подготовить путь для checks script.");
        return;
    }

    int fd = mkstemp(script_template);
    if (fd == -1)
    {
        log_error(app, "Не удалось создать временный checks script.");
        return;
    }

    FILE *f = fdopen(fd, "wb");
    if (!f)
    {
        close(fd);
        unlink(script_template);
        log_error(app, "Не удалось открыть checks script.");
        return;
    }

    int ok = write_base_checks(f);

    if (profile && profile_name_is_safe(profile->name))
    {
        ProfileConfig config;
        if (!profile_config_load(profile->name, &config))
        {
            fclose(f);
            unlink(script_template);
            log_error(app, "Не удалось загрузить профиль для checks: %s", profile->name);
            return;
        }

        ok = ok && write_profile_checks(f, profile, &config);
        profile_config_free(&config);
    }

    fclose(f);

    if (!ok)
    {
        unlink(script_template);
        log_error(app, "Не удалось записать checks script.");
        return;
    }

    if (chmod(script_template, 0755) != 0)
    {
        unlink(script_template);
        log_error(app, "Не удалось сделать checks script исполняемым.");
        return;
    }

    char cmd[MAX_CMD];
    int written = snprintf(cmd, sizeof(cmd), "bash \"%s\"", script_template);
    if (written < 0 || (size_t)written >= sizeof(cmd))
    {
        unlink(script_template);
        log_error(app, "Команда checks слишком длинная.");
        return;
    }

    log_info(app, "Проверка компонентов...");
    run_command_async(app, cmd);
}
