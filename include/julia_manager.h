#ifndef JULIA_MANAGER_H
#define JULIA_MANAGER_H

#include <stddef.h>

typedef enum JuliaChannel
{
    JULIA_CHANNEL_LTS = 0,
    JULIA_CHANNEL_STABLE = 1
} JuliaChannel;

typedef struct JuliaInstallRequest
{
    JuliaChannel channel;
} JuliaInstallRequest;

int julia_get_binary_path(char *out, size_t out_size);
int julia_is_installed(void);
int julia_is_link_present(void);
int julia_is_runtime_healthy(void);

int julia_build_install_command(
    const JuliaInstallRequest *request,
    char *out_cmd,
    size_t out_cmd_size
);

int julia_build_version_command(char *out_cmd, size_t out_cmd_size);

#endif