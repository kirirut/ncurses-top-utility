#ifndef GRAPH_H
#define GRAPH_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "process.h"

typedef struct {
    char *label;
} MenuItem;

typedef struct {
    char *label;
} Button;

void draw_menu(WINDOW *menu_win, MenuItem *items, int num_items, int highlight);
void draw_button(WINDOW *button_win, Button button, int highlight);
void draw_process_table(WINDOW *table_win, process *processes, size_t count);
void init_process_table(WINDOW *table_win);
void update_process_table(WINDOW *table_win, process *processes, size_t count);
int handle_menu(WINDOW *menu_win, MenuItem *menu_items, int num_menu_items);

#endif
