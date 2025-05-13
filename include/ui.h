#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include "process.h"


typedef struct {
    WINDOW *win;
    int height, width, starty, startx;
    int selected_process;
    int scroll_offset;
} UIWindow;



void init_ui(UIWindow *ui);
void cleanup_ui(UIWindow *ui);

#endif