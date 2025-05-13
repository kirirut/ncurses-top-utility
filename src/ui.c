#include "ui.h"

void init_ui(UIWindow *ui) {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100); // Set timeout to 100ms to make getch non-blocking

    ui->height = LINES - 3; // Leave space for top panel
    ui->width = COLS;
    ui->starty = 1;
    ui->startx = 0;
    ui->win = newwin(ui->height, ui->width, ui->starty, ui->startx);
    ui->selected_process = 0;
    ui->scroll_offset = 0;
    box(ui->win, 0, 0);
    refresh();
    wrefresh(ui->win);
}

void cleanup_ui(UIWindow *ui) {
    delwin(ui->win);
    endwin();
}