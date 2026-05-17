#ifndef INSTALL_STEPS_H
#define INSTALL_STEPS_H

#include "app.h"
#include "profile_config.h"

#include <stddef.h>

int build_install_command(
    const Profile *profile,
    const ProfileConfig *config,
    int update_mode,
    char *out_cmd,
    size_t out_cmd_size
);

int build_delete_python_env_command(
    const Profile *profile,
    char *out_cmd,
    size_t out_cmd_size
);

#endif