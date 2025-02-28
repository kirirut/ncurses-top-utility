#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"




int main() {
    initscr();  // Инициализация ncurses
    noecho();
    cbreak();
    curs_set(0);

    while (1) {
        clear();  // Очистка экрана

        draw_memory_bar();  // Отображение полоски для памяти
        draw_disk_bar();    // Отображение полоски для диска

        refresh();  // Обновление экрана
        usleep(1000000);  // Пауза 1 секунда (обновление каждые 1 секунду)
    }

    endwin();  // Завершение работы с ncurses
    return 0;
}
