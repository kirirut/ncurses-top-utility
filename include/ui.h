#ifndef UI_H
#define UI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "process.h"

typedef struct {
    WINDOW *win;          // Главное окно для вывода процессов
    int height;
    int width;
    int starty;
    int startx;
    int selected_process; // Для навигации по списку
    int scroll_offset;    // Смещение прокрутки
} UIWindow;

#endif