#ifndef JUPYTER_MANAGER_H
#define JUPYTER_MANAGER_H

#include <stddef.h>

int jupyter_get_binary_path(char *out, size_t out_size);
int jupyter_get_lab_binary_path(char *out, size_t out_size);

int jupyter_is_installed(void);
int jupyter_is_link_present(void);
int jupyter_is_runtime_healthy(void);

int jupyter_build_install_command(char *out_cmd, size_t out_cmd_size);
int jupyter_build_version_command(char *out_cmd, size_t out_cmd_size);
int jupyter_build_kernels_command(char *out_cmd, size_t out_cmd_size);
int jupyter_build_lab_command(char *out_cmd, size_t out_cmd_size);

#endif