#ifndef CHECKS_H
#define CHECKS_H

#include "app.h"

typedef enum InstallStatus
{
    INSTALL_STATUS_NOT_INSTALLED = 0,
    INSTALL_STATUS_INSTALLED_MATCH = 1,
    INSTALL_STATUS_INSTALLED_MISMATCH = 2,
    INSTALL_STATUS_BROKEN = 3,
    INSTALL_STATUS_READY = 4
} InstallStatus;

void run_component_checks(AppState *app);

#endif