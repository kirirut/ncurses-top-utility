#include "ui.h"
#include "process.h"
#include <time.h>

void init_ui(UIWindow *ui) {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

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

void update_top_panel(UIWindow *ui) {
    WINDOW *top = newwin(1, COLS, 0, 0);
    wattron(top, COLOR_PAIR(1));
    mvwprintw(top, 0, 0, "Mem: 2.85G/15.3G | Tasks: 145, 736 thr, 237 kthr, 1 running | Load average: 0.73 0.99 0.58 | Uptime: 00:07:35");
    wattroff(top, COLOR_PAIR(1));
    wrefresh(top);
    delwin(top);
}

void display_processes(UIWindow *ui, process *p_list, size_t count, SortCriterion criterion) {
    werase(ui->win);
    box(ui->win, 0, 0);

    int items_per_page = ui->height - 2;
    int total_pages = (count + items_per_page - 1) / items_per_page;
    int current_page = ui->scroll_offset / items_per_page;

    for (int i = ui->scroll_offset; i < count && i < ui->scroll_offset + items_per_page; i++) {
        if (i == ui->selected_process) {
            wattron(ui->win, A_REVERSE);
        }
        mvwprintw(ui->win, i - ui->scroll_offset + 1, 1, "PID: %d | Name: %s | CPU: %.2f%% | Memory: %lu KB",
                  p_list[i].pid, p_list[i].name, p_list[i].cpu_usage, p_list[i].memory);
        wattroff(ui->win, A_REVERSE);
    }

    mvwprintw(ui->win, 0, 1, "Sorted by: %s | Page %d/%d", 
              criterion == SORT_BY_PID ? "PID" : criterion == SORT_BY_NAME ? "Name" : 
              criterion == SORT_BY_CPU ? "CPU" : "Memory", current_page + 1, total_pages);
    wrefresh(ui->win);
}

void display_help_panel(UIWindow *ui) {
    WINDOW *help = newwin(10, 40, (ui->height - 10) / 2 + ui->starty, (ui->width - 40) / 2);
    box(help, 0, 0);
    mvwprintw(help, 1, 1, "Help - Key Bindings");
    mvwprintw(help, 2, 1, "-----------------");
    mvwprintw(help, 3, 1, "Up/Down: Navigate");
    mvwprintw(help, 4, 1, "PgUp/PgDn: Scroll");
    mvwprintw(help, 5, 1, "p: Sort by PID");
    mvwprintw(help, 6, 1, "n: Sort by Name");
    mvwprintw(help, 7, 1, "c: Sort by CPU");
    mvwprintw(help, 8, 1, "m: Sort by Memory");
    mvwprintw(help, 9, 1, "q: Quit");
    wrefresh(help);
    getch();
    delwin(help);
    touchwin(ui->win);
    wrefresh(ui->win);
}

void handle_input(UIWindow *ui, process *p_list, size_t count) {
    int ch;
    SortCriterion criterion = SORT_BY_PID;
    time_t last_refresh = time(NULL);

    while ((ch = getch()) != 'q') {
        time_t current_time = time(NULL);
        if (difftime(current_time, last_refresh) >= 15) {
            if (parse_processes(p_list, count, criterion)) {
                display_processes(ui, p_list, count, criterion);
            }
            last_refresh = current_time;
        }

        switch (ch) {
            case KEY_UP:
                if (ui->selected_process > 0) ui->selected_process--;
                if (ui->selected_process < ui->scroll_offset) ui->scroll_offset = ui->selected_process;
                break;
            case KEY_DOWN:
                if (ui->selected_process < (int)count - 1) ui->selected_process++;
                if (ui->selected_process >= ui->scroll_offset + ui->height - 2) ui->scroll_offset++;
                break;
            case KEY_PPAGE:
                ui->scroll_offset -= ui->height - 2;
                if (ui->scroll_offset < 0) ui->scroll_offset = 0;
                ui->selected_process = ui->scroll_offset;
                break;
            case KEY_NPAGE:
                ui->scroll_offset += ui->height - 2;
                if (ui->scroll_offset >= (int)count) ui->scroll_offset = count - 1;
                ui->selected_process = ui->scroll_offset;
                break;
            case 'p':
                criterion = SORT_BY_PID;
                qsort(p_list, count, sizeof(process), compare_processes(criterion));
                ui->selected_process = 0;
                ui->scroll_offset = 0;
                break;
            case 'n':
                criterion = SORT_BY_NAME;
                qsort(p_list, count, sizeof(process), compare_processes(criterion));
                ui->selected_process = 0;
                ui->scroll_offset = 0;
                break;
            case 'c':
                criterion = SORT_BY_CPU;
                qsort(p_list, count, sizeof(process), compare_processes(criterion));
                ui->selected_process = 0;
                ui->scroll_offset = 0;
                break;
            case 'm':
                criterion = SORT_BY_MEMORY;
                qsort(p_list, count, sizeof(process), compare_processes(criterion));
                ui->selected_process = 0;
                ui->scroll_offset = 0;
                break;
            case KEY_F(1):
                display_help_panel(ui);
                break;
        }
        update_top_panel(ui);
        display_processes(ui, p_list, count, criterion);
    }
}

void cleanup_ui(UIWindow *ui) {
    delwin(ui->win);
    endwin();
}