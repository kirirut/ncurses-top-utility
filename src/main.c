#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"
#include "process.h"

#define MENU_BAR_HEIGHT 3

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Menu
    MenuItem menu_items[] = {
        {"Help"},
        {"Search"},
        {"Filter"},
        {"Sort"},
        {"Options"},
        {"Kill"},
        {"Quit"}
    };
    int num_menu_items = sizeof(menu_items) / sizeof(menu_items[0]);
    WINDOW *menu_win = subwin(stdscr, MENU_BAR_HEIGHT, max_x, max_y - MENU_BAR_HEIGHT, 0);
    if (menu_win == NULL) {
        endwin();
        fprintf(stderr, "Error creating subwindow\n");
        return 1;
    }
    keypad(menu_win, TRUE);
    wbkgd(menu_win, COLOR_PAIR(1));
    box(menu_win, 0, 0);
    draw_menu_bar(menu_win, menu_items, num_menu_items);

    // Process table
    WINDOW *table_win = subwin(stdscr, max_y - MENU_BAR_HEIGHT - 1, max_x, 1, 0);
    if (table_win == NULL) {
        endwin();
        fprintf(stderr, "Error creating subwindow\n");
        return 1;
    }
    keypad(table_win, TRUE);

    size_t process_count;
    process *p_list = NULL;

    int choice;
    while (1) {
        process_count = get_process_count();
        p_list = allocate_processes(process_count);
        parse_processes(p_list, process_count);
        update_process_table(table_win, p_list, process_count);
        choice = handle_menu(menu_win, menu_items, num_menu_items);
        if (choice == 1) {
            // Help
            display_help_window();
            draw_menu_bar(menu_win, menu_items, num_menu_items);
        } else if (choice == 2) {
            // Search
            mvwprintw(stdscr, 0, 0, "Search selected");
        } else if (choice == 3) {
            // Filter
            mvwprintw(stdscr, 0, 0, "Filter selected");
        } else if (choice == 4) {
            // Sort
            mvwprintw(stdscr, 0, 0, "Sort selected");
        } else if (choice == 5) {
            // Options
            mvwprintw(stdscr, 0, 0, "Options selected");
        } else if (choice == 6) {
            // Kill
            mvwprintw(stdscr, 0, 0, "Kill selected");
        } else if (choice == 7) {
            // Exit
            mvwprintw(stdscr, 0, 0, "Exit selected");
            break;
        }
        refresh();
        free_processes(&p_list, process_count);
    }

    endwin();
    return 0;
}
