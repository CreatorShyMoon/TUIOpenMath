#ifndef SCILAB_MANAGER_H
#define SCILAB_MANAGER_H

#include <stddef.h>

int scilab_build_install_command(char *out_cmd, size_t out_cmd_size);
int scilab_build_remove_command(char *out_cmd, size_t out_cmd_size);
int scilab_build_register_kernel_command(char *out_cmd, size_t out_cmd_size);

#endif