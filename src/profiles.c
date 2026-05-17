#include "profiles.h"
#include "logger.h"
#include "profile_config.h"
#include "profile_registry.h"

#include <string.h>

int profile_name_is_safe(const char *name)
{
    return profile_config_name_is_valid(name);
}

int profile_is_user_editable(const Profile *profile)
{
    (void)profile;
    return 0;
}

static void clear_profiles(AppState *app)
{
    app->profile_count = 0;
    memset(app->profiles, 0, sizeof(app->profiles));
}

int load_profiles(AppState *app, const char *dir_path)
{
    (void)dir_path;

    if (!app)
    {
        return 0;
    }

    clear_profiles(app);

    for (size_t i = 0; i < BUILTIN_PROFILES_COUNT; i++)
    {
        if (app->profile_count >= MAX_PROFILES)
        {
            log_warn(app, "Достигнут лимит профилей: %d", MAX_PROFILES);
            break;
        }

        const BuiltinProfile *bp = &BUILTIN_PROFILES[i];

        if (!profile_config_name_is_valid(bp->name))
        {
            log_warn(app, "Профиль пропущен: небезопасное имя '%s'", bp->name);
            continue;
        }

        Profile profile;
        memset(&profile, 0, sizeof(profile));

        snprintf(profile.name,        sizeof(profile.name),        "%s", bp->name);
        snprintf(profile.description, sizeof(profile.description), "%s", bp->description);
        snprintf(profile.path,        sizeof(profile.path),        "(builtin)");

        profile.is_system_profile = 1;
        profile.is_locked         = bp->locked;

        app->profiles[app->profile_count++] = profile;
    }

    if (app->profile_count == 0)
    {
        log_error(app, "Профили не найдены в реестре.");
        return 0;
    }

    app->selected_profile = 0;
    return 1;
}

const Profile *get_selected_profile(const AppState *app)
{
    if (!app || app->profile_count <= 0)
    {
        return NULL;
    }

    if (app->selected_profile < 0 || app->selected_profile >= app->profile_count)
    {
        return NULL;
    }

    return &app->profiles[app->selected_profile];
}