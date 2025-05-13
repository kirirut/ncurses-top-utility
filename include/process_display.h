#ifndef PROCESS_DISPLAY_H
#define PROCESS_DISPLAY_H

#include "ui.h"

void display_processes(UIWindow *ui, process *p_list, size_t count, SortCriterion criterion);
void display_process_details_panel(UIWindow *ui, process *proc);

#endif