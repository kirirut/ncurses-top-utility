#include "graph.h"

void draw_menu(WINDOW *menu_win, MenuItem *items, int num_items, int highlight) {
    int x, y, i;

    x = 2;
    y = 2;
    box(menu_win, 0, 0);
    for (i = 0; i < num_items; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
        }
        mvwprintw(menu_win, y, x, items[i].label);
        wattroff(menu_win, A_REVERSE);
        ++y;
    }
    wrefresh(menu_win);
}

void draw_button(WINDOW *button_win, Button button, int highlight) {
    int x, y;
    x = 2;
    y = 1;

    box(button_win, 0, 0);
    if (highlight) {
        wattron(button_win, A_REVERSE);
    }
    mvwprintw(button_win, y, x, button.label);
    wattroff(button_win, A_REVERSE);
    wrefresh(button_win);
}

int handle_menu(WINDOW *menu_win, MenuItem *menu_items, int num_menu_items) {
    int highlight = 1;
    int choice = 0;
    int c;
    draw_menu(menu_win, menu_items, num_menu_items, highlight);

    while (1) {
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = num_menu_items;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == num_menu_items)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            case KEY_RESIZE:
                // Handle window resize
                int menu_height, menu_width;
                getmaxyx(stdscr, menu_height, menu_width);
                wresize(menu_win, menu_height, menu_width);
                mvwin(menu_win, 0, 0);
                wclear(menu_win);
                break;
            default:
                break;
        }
        draw_menu(menu_win, menu_items, num_menu_items, highlight);
        if (choice != 0) {
            return choice;
        }
    }
}
