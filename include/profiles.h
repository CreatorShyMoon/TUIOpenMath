#ifndef PROFILES_H
#define PROFILES_H

#include "app.h"

int load_profiles(AppState *app, const char *dir_path);
int profile_name_is_safe(const char *name);
int profile_is_user_editable(const Profile *profile);
const Profile *get_selected_profile(const AppState *app);

#endif
