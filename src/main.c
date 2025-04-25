#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Menu items
    MenuItem menu_items[] = {
        {"Help"},
        {"Search"},
        {"Filter"},
        {"Sort"},
        {"Options"},
        {"Kill"},
        {"Exit"}
    };
    int num_menu_items = sizeof(menu_items) / sizeof(menu_items[0]);
    int menu_height, menu_width, menu_start_y, menu_start_x;
    getmaxyx(stdscr, menu_height, menu_width);
    menu_start_y = 0;
    menu_start_x = 0;
    WINDOW *menu_win = newwin(menu_height, menu_width, menu_start_y, menu_start_x);
    keypad(menu_win, TRUE);

    int choice;
    while (1) {
        choice = handle_menu(menu_win, menu_items, num_menu_items);
        if (choice == 1) {
            // Help
            mvwprintw(stdscr, 0, 0, "Help selected");
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
        }
        else if (choice == 6) {
            // Options
            mvwprintw(stdscr, 0, 0, "Kill selected");
        } 
        else if (choice == 7) {
            // Exit
            mvwprintw(stdscr, 0, 0, "Exit selected");
            break;
        }
        refresh();
    }

    endwin();
    return 0;
}
