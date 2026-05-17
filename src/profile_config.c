#include "profile_config.h"
#include "profile_registry.h"

#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int string_list_push(StringList *list, const char *value);
static int parse_string_array(cJSON *node, StringList *list);

static int parse_string_array(cJSON *node, StringList *list)
{
    if (!node)
    {
        return 1;
    }

    if (!cJSON_IsArray(node))
    {
        return 0;
    }

    int size = cJSON_GetArraySize(node);
    for (int i = 0; i < size; i++)
    {
        cJSON *item = cJSON_GetArrayItem(node, i);
        if (!cJSON_IsString(item) || !item->valuestring)
        {
            return 0;
        }

        if (!string_list_push(list, item->valuestring))
        {
            return 0;
        }
    }

    return 1;
}

static void profile_config_init(ProfileConfig *config)
{
    if (!config)
    {
        return;
    }

    memset(config->name, 0, sizeof(config->name));
    config->is_system_profile = 0;
    config->is_locked = 0;

    config->python.items = NULL;
    config->python.count = 0;

    config->julia.items = NULL;
    config->julia.count = 0;

    config->system.items = NULL;
    config->system.count = 0;

    config->kernels.items = NULL;
    config->kernels.count = 0;
}

static char *dup_string(const char *src)
{
    if (!src)
    {
        return NULL;
    }

    size_t len = strlen(src);
    char *copy = malloc(len + 1);
    if (!copy)
    {
        return NULL;
    }

    memcpy(copy, src, len + 1);
    return copy;
}

int profile_config_load(const char *profile_name, ProfileConfig *config)
{
    if (!profile_name || !config)
    {
        return 0;
    }

    profile_config_init(config);

    const BuiltinProfile *builtin =
        find_builtin_profile(profile_name);

    if (!builtin)
    {
        return 0;
    }

    cJSON *json = cJSON_Parse(builtin->json);

    if (!json)
    {
        return 0;
    }

    cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
    cJSON *python = cJSON_GetObjectItemCaseSensitive(json, "python");
    cJSON *julia  = cJSON_GetObjectItemCaseSensitive(json, "julia");
    cJSON *system = cJSON_GetObjectItemCaseSensitive(json, "system");
    cJSON *kernels = cJSON_GetObjectItemCaseSensitive(json, "kernels");

    if (cJSON_IsString(name) && name->valuestring)
    {
        snprintf(config->name, sizeof(config->name), "%s", name->valuestring);
    }

    config->is_system_profile = 1;
    config->is_locked = 1;

    int ok =
        parse_string_array(python, &config->python) &&
        parse_string_array(julia, &config->julia) &&
        parse_string_array(system, &config->system) &&
        parse_string_array(kernels, &config->kernels);

    cJSON_Delete(json);

    if (!ok || !profile_config_validate(config))
    {
        profile_config_free(config);
        return 0;
    }

    return 1;
}

int profile_config_name_is_valid(const char *name)
{
    if (!name || name[0] == '\0')
    {
        return 0;
    }

    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
    {
        unsigned char ch = *p;

        if ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_')
        {
            continue;
        }

        return 0;
    }

    return 1;
}

static int token_is_valid(const char *value)
{
    if (!value || value[0] == '\0')
    {
        return 0;
    }

    for (const unsigned char *p = (const unsigned char *)value; *p; ++p)
    {
        unsigned char ch = *p;

        if ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_' || ch == '.' || ch == '+')
        {
            continue;
        }

        return 0;
    }

    return 1;
}

static int string_list_contains(const StringList *list, const char *value)
{
    if (!list || !value)
    {
        return 0;
    }

    for (size_t i = 0; i < list->count; i++)
    {
        if (strcmp(list->items[i], value) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int string_list_push(StringList *list, const char *value)
{
    if (!list || !value)
    {
        return 0;
    }

    if (!token_is_valid(value))
    {
        return 0;
    }

    if (string_list_contains(list, value))
    {
        return 1;
    }

    char **new_items = realloc(list->items, (list->count + 1) * sizeof(char *));
    if (!new_items)
    {
        return 0;
    }

    list->items = new_items;
    list->items[list->count] = dup_string(value);
    if (!list->items[list->count])
    {
        return 0;
    }

    list->count++;
    return 1;
}




static int string_equals_any(const char *value, const char *const *allowed, size_t allowed_count)
{
    if (!value || !allowed)
    {
        return 0;
    }

    for (size_t i = 0; i < allowed_count; i++)
    {
        if (strcmp(value, allowed[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int validate_kernel_list(const StringList *list)
{
    static const char *const ALLOWED[] = {
        "python",
        "julia",
        "bash",
        "octave",
        "scilab"
    };

    if (!list)
    {
        return 0;
    }

    for (size_t i = 0; i < list->count; i++)
    {
        if (!string_equals_any(list->items[i], ALLOWED, sizeof(ALLOWED) / sizeof(ALLOWED[0])))
        {
            return 0;
        }
    }

    return 1;
}

static int validate_system_list(const StringList *list)
{
    static const char *const ALLOWED[] = {
        "curl",
        "git",
        "python3-venv",
        "python3-pip",
        "octave",
        "scilab"
    };

    if (!list)
    {
        return 0;
    }

    for (size_t i = 0; i < list->count; i++)
    {
        if (!string_equals_any(list->items[i], ALLOWED, sizeof(ALLOWED) / sizeof(ALLOWED[0])))
        {
            return 0;
        }
    }

    return 1;
}

int profile_config_validate(const ProfileConfig *config)
{
    if (!config)
    {
        return 0;
    }

    if (!profile_config_name_is_valid(config->name))
    {
        return 0;
    }

    if (!validate_system_list(&config->system))
    {
        return 0;
    }

    if (!validate_kernel_list(&config->kernels))
    {
        return 0;
    }

    return 1;
}


void profile_config_free(ProfileConfig *config)
{
    if (!config)
    {
        return;
    }

    StringList *lists[] = {
        &config->python,
        &config->julia,
        &config->system,
        &config->kernels
    };

    for (size_t i = 0; i < sizeof(lists) / sizeof(lists[0]); i++)
    {
        for (size_t j = 0; j < lists[i]->count; j++)
        {
            free(lists[i]->items[j]);
        }

        free(lists[i]->items);
        lists[i]->items = NULL;
        lists[i]->count = 0;
    }

    memset(config->name, 0, sizeof(config->name));
    config->is_system_profile = 0;
    config->is_locked = 0;
}
