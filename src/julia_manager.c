#include "julia_manager.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define JULIA_BIN_PATH "tools/bin/julia"
#define JULIA_ROOT_DIR "tools/julia"
#define JULIA_BIN_DIR  "tools/bin"

int lstat(const char *path, struct stat *buf);

static int extract_major_minor(const char *version, char *out, size_t out_size)
{
    if (!version || !out || out_size == 0)
    {
        return 0;
    }

    const char *last_dot = strrchr(version, '.');
    if (!last_dot)
    {
        return 0;
    }

    size_t len = (size_t)(last_dot - version);
    if (len == 0 || len >= out_size)
    {
        return 0;
    }

    memcpy(out, version, len);
    out[len] = '\0';
    return 1;
}

int julia_get_binary_path(char *out, size_t out_size)
{
    if (!out || out_size == 0)
    {
        return 0;
    }

    int written = snprintf(out, out_size, "%s", JULIA_BIN_PATH);
    return written >= 0 && (size_t)written < out_size;
}

int julia_is_link_present(void)
{
    struct stat st;
    return lstat(JULIA_BIN_PATH, &st) == 0;
}

int julia_is_installed(void)
{
    return access(JULIA_BIN_PATH, X_OK) == 0;
}

int julia_is_runtime_healthy(void)
{
    struct stat st;
    if (lstat(JULIA_BIN_PATH, &st) != 0)
    {
        return 0;
    }

    if (access(JULIA_BIN_PATH, X_OK) != 0)
    {
        return 0;
    }

    return 1;
}

int julia_build_version_command(char *out_cmd, size_t out_cmd_size)
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
        "  echo \"[FAIL] Julia symlink is broken: %s\"; "
        "elif [ -x \"%s\" ]; then "
        "  \"%s\" --version; "
        "else "
        "  echo \"Julia not installed\"; "
        "fi"
        "'",
        JULIA_BIN_PATH,
        JULIA_BIN_PATH,
        JULIA_BIN_PATH,
        JULIA_BIN_PATH,
        JULIA_BIN_PATH
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int julia_build_install_command(
    const JuliaInstallRequest *request,
    char *out_cmd,
    size_t out_cmd_size
)
{
    if (!request || !out_cmd || out_cmd_size == 0)
    {
        return 0;
    }

    const char *version = NULL;
    const char *label = NULL;

    switch (request->channel)
    {
        case JULIA_CHANNEL_LTS:
            version = "1.10.10";
            label = "lts";
            break;
        case JULIA_CHANNEL_STABLE:
            version = "1.11.7";
            label = "stable";
            break;
        default:
            return 0;
    }

    char major_minor[16];
    if (!extract_major_minor(version, major_minor, sizeof(major_minor)))
    {
        return 0;
    }

    int written = snprintf(
        out_cmd,
        out_cmd_size,
        "bash -lc '"
        "set -euo pipefail; "
        "if ! command -v curl >/dev/null 2>&1; then echo \"[ERROR] curl не найден\" >&2; exit 127; fi; "
        "if ! command -v tar >/dev/null 2>&1; then echo \"[ERROR] tar не найден\" >&2; exit 127; fi; "
        "if [ -x \"%s\" ]; then "
        "  JULIA_VERSION_LINE=$(\"%s\" --version 2>/dev/null | head -n 1); "
        "  set -- $JULIA_VERSION_LINE; "
        "  CURRENT_VERSION=${3:-}; "
        "  if [ \"${CURRENT_VERSION:-}\" = \"%s\" ]; then "
        "    echo \"[INFO] Julia %s already installed\"; "
        "    \"%s\" --version; "
        "    exit 0; "
        "  else "
        "    echo \"[WARN] Julia version mismatch: found ${CURRENT_VERSION:-unknown}, required %s\"; "
        "  fi; "
        "fi; "
        "mkdir -p \"%s\" \"%s\"; "
        "ARCHIVE=\"julia-%s-linux-x86_64.tar.gz\"; "
        "URL=\"https://julialang-s3.julialang.org/bin/linux/x64/%s/$ARCHIVE\"; "
        "TARGET_DIR=\"%s/%s\"; "
        "TMP_ARCHIVE=$(mktemp /tmp/sysopenmath_julia_XXXXXX.tar.gz); "
        "rm -rf \"$TARGET_DIR\"; "
        "mkdir -p \"$TARGET_DIR\"; "
        "echo \"[STEP] download Julia %s\"; "
        "curl -fL \"$URL\" -o \"$TMP_ARCHIVE\"; "
        "echo \"[STEP] extract Julia %s\"; "
        "tar -xzf \"$TMP_ARCHIVE\" -C \"$TARGET_DIR\" --strip-components=1; "
        "rm -f \"$TMP_ARCHIVE\"; "
        "ln -sfn \"$(pwd)/$TARGET_DIR/bin/julia\" \"%s\"; "
        "echo \"[INFO] Julia %s installed\"; "
        "\"%s\" --version"
        "'",
        JULIA_BIN_PATH,
        JULIA_BIN_PATH,
        version,
        version,
        JULIA_BIN_PATH,
        version,
        JULIA_ROOT_DIR,
        JULIA_BIN_DIR,
        version,
        major_minor,
        JULIA_ROOT_DIR,
        label,
        version,
        version,
        JULIA_BIN_PATH,
        version,
        JULIA_BIN_PATH
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}

int julia_build_remove_command(char *out_cmd, size_t out_cmd_size)
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
        "if [ -L \"%s\" ] || [ -e \"%s\" ] || [ -d \"%s\" ]; then "
        "  rm -f \"%s\"; "
        "  rm -rf \"%s\"; "
        "  echo \"[INFO] Julia removed\"; "
        "else "
        "  echo \"[WARN] Julia not installed\"; "
        "fi"
        "'",
        JULIA_BIN_PATH,
        JULIA_BIN_PATH,
        JULIA_ROOT_DIR,
        JULIA_BIN_PATH,
        JULIA_ROOT_DIR
    );

    return written >= 0 && (size_t)written < out_cmd_size;
}
