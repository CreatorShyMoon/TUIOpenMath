#ifndef CLEANUP_MANAGER_H
#define CLEANUP_MANAGER_H

#include "app.h"

#include <stddef.h>

int cleanup_build_remove_julia_command(char *out_cmd, size_t out_cmd_size);
int cleanup_build_remove_jupyter_command(char *out_cmd, size_t out_cmd_size);
int cleanup_build_clear_kernels_command(char *out_cmd, size_t out_cmd_size);
int cleanup_build_clear_state_command(char *out_cmd, size_t out_cmd_size);

#endif