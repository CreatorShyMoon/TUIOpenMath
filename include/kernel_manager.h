#ifndef KERNEL_MANAGER_H
#define KERNEL_MANAGER_H

#include "app.h"

#include <stddef.h>

int kernel_build_register_python_command(
    const Profile *profile,
    char *out_cmd,
    size_t out_cmd_size
);

int kernel_build_register_julia_command(
    char *out_cmd,
    size_t out_cmd_size
);

int kernel_build_register_bash_command(
    char *out_cmd,
    size_t out_cmd_size
);

int kernel_build_register_octave_command(
    char *out_cmd,
    size_t out_cmd_size
);

#endif