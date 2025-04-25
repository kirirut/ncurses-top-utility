#include "graph.h"
#define MENU_BAR_HEIGHT 3

void draw_menu_bar(WINDOW *menu_win, MenuItem *items, int num_items) {
    int x, y, i;
    int max_x, max_y;

    getmaxyx(menu_win, max_y, max_x);
    x = 1;
    y = 1;

    for (i = 0; i < num_items; ++i) {
        mvwprintw(menu_win, y, x, items[i].label);
        x += strlen(items[i].label) + 3; // Add some space between items
    }
    wrefresh(menu_win);
}

void draw_menu(WINDOW *menu_win, MenuItem *items, int num_items, int highlight) {
    int x, y, i;

    x = 1;
    y = 1;
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
    //draw_menu(menu_win, menu_items, num_menu_items, highlight); // No longer needed

    while (1) {
        c = wgetch(menu_win);
        switch (c) {
            case KEY_LEFT:
                if (highlight == 1)
                    highlight = num_menu_items;
                else
                    --highlight;
                break;
            case KEY_RIGHT:
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
                wresize(menu_win, MENU_BAR_HEIGHT, menu_width);
                mvwin(menu_win, LINES - MENU_BAR_HEIGHT, 0);
                wclear(menu_win);
                draw_menu_bar(menu_win, menu_items, num_menu_items);
                break;
            default:
                break;
        }
        //draw_menu(menu_win, menu_items, num_menu_items, highlight); // No longer needed
        if (choice != 0) {
            return choice;
        }
        int x = 1;
        for (int i = 0; i < num_menu_items; ++i) {
            if (highlight == i + 1) {
                wattron(menu_win, A_REVERSE);
            }
            mvwprintw(menu_win, 1, x, menu_items[i].label);
            wattroff(menu_win, A_REVERSE);
            x += strlen(menu_items[i].label) + 3;
        }
        wrefresh(menu_win);
    }
}

// Function to display the help window, similar to htop's style
void display_help_window() {
    WINDOW *help_win;
    int help_height = 20; // Increased height for more content
    int help_width = 60;  // Increased width for better readability
    int help_start_y = (LINES - help_height) / 2;
    int help_start_x = (COLS - help_width) / 2;

    // Create the help window
    help_win = newwin(help_height, help_width, help_start_y, help_start_x);
    if (help_win == NULL) {
        // Handle error if window creation fails
        mvwprintw(stdscr, 0, 0, "Error creating help window");
        refresh();
        return;
    }

    // Draw a box around the help window
    box(help_win, 0, 0);

    // Help window title
    mvwprintw(help_win, 1, 2, "ntop Help (Press 'q' or 'Esc' to exit)");
    mvwprintw(help_win, 2, 2, "--------------------------------------------------");

    // Help content
    mvwprintw(help_win, 4, 2, "General Commands:");
    mvwprintw(help_win, 5, 4, "Up/Down: Navigate through the process list");
    mvwprintw(help_win, 6, 4, "Left/Right: Navigate through the menu");
    mvwprintw(help_win, 7, 4, "Enter: Select an option from the menu");
    mvwprintw(help_win, 8, 4, "F1 or ?: Display this help window");
    mvwprintw(help_win, 9, 4, "q or Esc: Quit ntop");

    mvwprintw(help_win, 11, 2, "Menu Commands:");
    mvwprintw(help_win, 12, 4, "Help: Display this help window");
    mvwprintw(help_win, 13, 4, "Search: Search for a process");
    mvwprintw(help_win, 14, 4, "Filter: Filter processes");
    mvwprintw(help_win, 15, 4, "Sort: Sort processes by different criteria");
    mvwprintw(help_win, 16, 4, "Options: Configure ntop");
    mvwprintw(help_win, 17, 4, "Kill: Kill a selected process");
    mvwprintw(help_win, 18, 4, "Quit: Exit ntop");

    wrefresh(help_win);

    int ch;
    while ((ch = wgetch(help_win)) != 'q' && ch != 27) { // Wait for 'q' or 'Esc'
        // Do nothing, just wait for the user to press 'q' or 'Esc'
    }

    // Clean up and refresh the main screen
    delwin(help_win);
    touchwin(stdscr);
    refresh();
}
void init_process_table(WINDOW *table_win) {
    box(table_win, 0, 0);
    mvwprintw(table_win, 0, 2, "Processes");
    mvwprintw(table_win, 1, 1, "PID");
    mvwprintw(table_win, 1, 8, "Name");
    mvwprintw(table_win, 1, 30, "State");
    mvwprintw(table_win, 1, 38, "Memory");
    mvwprintw(table_win, 1, 48, "CPU");
    wrefresh(table_win);
}

void update_process_table(WINDOW *table_win, process *processes, size_t count) {
    wclear(table_win);
    init_process_table(table_win);
    int y = 2;
    for (size_t i = 0; i < count; i++) {
        mvwprintw(table_win, y, 1, "%d", processes[i].pid);
        mvwprintw(table_win, y, 8, "%s", processes[i].name);
        mvwprintw(table_win, y, 30, "%c", processes[i].state);
        mvwprintw(table_win, y, 38, "%lu", processes[i].memory);
        mvwprintw(table_win, y, 48, "%.2f", processes[i].cpu_usage);
        y++;
    }
    wrefresh(table_win);
}

void draw_process_table(WINDOW *table_win, process *processes, size_t count) {
    
    init_process_table(table_win);
    update_process_table(table_win, processes, count);
}
