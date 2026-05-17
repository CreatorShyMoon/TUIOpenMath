#ifndef ACTIONS_H
#define ACTIONS_H

#include "app.h"

int perform_install_profile(AppState *app);
int perform_update_profile(AppState *app);
int perform_delete_environment(AppState *app);

int perform_install_julia_lts(AppState *app);
int perform_install_julia_stable(AppState *app);
int perform_show_julia_version(AppState *app);

int perform_install_jupyter(AppState *app);
int perform_show_jupyter_version(AppState *app);
int perform_show_jupyter_kernels(AppState *app);
int perform_start_jupyterlab(AppState *app);

int perform_register_python_kernel(AppState *app);
int perform_register_julia_kernel(AppState *app);

int perform_stop_jupyterlab(AppState *app);
int perform_restart_jupyterlab(AppState *app);
int perform_show_state(AppState *app);

int perform_remove_julia(AppState *app);
int perform_remove_jupyter(AppState *app);
int perform_clear_kernels(AppState *app);
int perform_clear_state(AppState *app);

int perform_create_workspace(AppState *app);

void perform_reload_profiles(AppState *app);
void perform_selected_action(AppState *app);

int perform_register_bash_kernel(AppState *app);
int perform_register_octave_kernel(AppState *app);

int perform_full_setup_selected(AppState *app);

int perform_install_octave(AppState *app);
int perform_remove_octave(AppState *app);

int perform_install_scilab(AppState *app);
int perform_remove_scilab(AppState *app);
int perform_register_scilab_kernel(AppState *app);


#endif