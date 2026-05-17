#ifndef PROFILE_REGISTRY_H
#define PROFILE_REGISTRY_H

#include <stddef.h>

typedef struct
{
    const char *name;
    const char *description;
    const char *json;
    int locked;

} BuiltinProfile;

extern const BuiltinProfile BUILTIN_PROFILES[];
extern const size_t BUILTIN_PROFILES_COUNT;

const BuiltinProfile *find_builtin_profile(const char *name);

#endif