#ifndef PANELS_H
#define PANELS_H

#include "ui.h"

void display_help_panel(UIWindow *ui);
int display_confirmation_panel(UIWindow *ui, int pid, const char *name);
void display_core_stats_panel(UIWindow *ui);

#endif