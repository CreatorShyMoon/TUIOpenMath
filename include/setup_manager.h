#ifndef SETUP_MANAGER_H
#define SETUP_MANAGER_H

#include "app.h"

#include <stddef.h>

int setup_build_full_profile_command(
    const Profile *profile,
    char *out_cmd,
    size_t out_cmd_size
);

#endif
