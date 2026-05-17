#ifndef OCTAVE_MANAGER_H
#define OCTAVE_MANAGER_H

#include <stddef.h>

int octave_build_install_command(char *out_cmd, size_t out_cmd_size);
int octave_build_remove_command(char *out_cmd, size_t out_cmd_size);

#endif
