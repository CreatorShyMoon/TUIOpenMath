#ifndef PROFILE_CONFIG_H
#define PROFILE_CONFIG_H

#include <stddef.h>

typedef struct StringList
{
    char **items;
    size_t count;
} StringList;

typedef struct ProfileConfig
{
    char name[64];
    int is_system_profile;
    int is_locked;

    StringList python;
    StringList julia;
    StringList system;
    StringList kernels;
} ProfileConfig;

int profile_config_name_is_valid(const char *name);
int profile_config_load(const char *path, ProfileConfig *config);
int profile_config_validate(const ProfileConfig *config);
void profile_config_free(ProfileConfig *config);

#endif
