#include "panels.h"
#include <string.h>
#include <stdio.h>

void display_help_panel(UIWindow *ui) {
    int help_width = 70; // Widened window to ensure "F9: Kill Process" fits
    int help_height = 13;
    WINDOW *help = newwin(help_height, help_width, (ui->height - help_height) / 2 + ui->starty, (ui->width - help_width) / 2);
    box(help, 0, 0);

    // Center the title and make it bold
    const char *title = "Help - Key Bindings";
    int title_len = strlen(title);
    int title_x = (help_width - title_len) / 2;
    wattron(help, A_BOLD);
    mvwprintw(help, 1, title_x, "%s", title);
    wattroff(help, A_BOLD);

    // Solid separator line
    for (int i = 1; i < help_width - 1; i++) {
        mvwprintw(help, 2, i, "_");
    }

    mvwprintw(help, 3, 1, "Up/Down: Navigate");
    mvwprintw(help, 4, 1, "PgUp/PgDn: Scroll");
    mvwprintw(help, 5, 1, "p: Sort by PID");
    mvwprintw(help, 6, 1, "n: Sort by Name");
    mvwprintw(help, 7, 1, "c: Sort by CPU");
    mvwprintw(help, 8, 1, "m: Sort by Memory");
    mvwprintw(help, 9, 1, "F2: Core Stats");
    mvwprintw(help, 10, 1, "F3: Process Details");
    mvwprintw(help, 11, 1, "F9: Kill Process");
    wrefresh(help);

    // Temporarily set blocking mode to wait for key press
    timeout(-1); // -1 means blocking
    getch();
    timeout(100); // Restore non-blocking mode

    delwin(help);
    touchwin(ui->win);
    wrefresh(ui->win);
}

int display_confirmation_panel(UIWindow *ui, int pid, const char *name) {
    WINDOW *confirm = newwin(5, 40, (ui->height - 5) / 2 + ui->starty, (ui->width - 40) / 2);
    box(confirm, 0, 0);

    // Center the "Kill process" message
    char message[256];
    snprintf(message, sizeof(message), "Kill process %d (%s)?", pid, name);
    int message_len = strlen(message);
    int message_x = (40 - message_len) / 2; // Center the message in a 40-char wide window
    mvwprintw(confirm, 1, message_x, "%s", message);

    // Position the buttons symmetrically
    int selected = 0; // 0 for Yes, 1 for No
    int confirmed = -1; // -1 for no selection, 0 for No, 1 for Yes

    while (confirmed == -1) {
        // Display buttons centered with equal spacing
        if (selected == 0) wattron(confirm, A_REVERSE);
        mvwprintw(confirm, 3, 10, "[ Yes ]"); // Positioned at x=10
        wattroff(confirm, A_REVERSE);

        if (selected == 1) wattron(confirm, A_REVERSE);
        mvwprintw(confirm, 3, 23, "[ No ]"); // Positioned at x=23 (10 + 7 for "[ Yes ]" + 6 spaces)
        wattroff(confirm, A_REVERSE);

        wrefresh(confirm);

        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                selected = 0;
                break;
            case KEY_RIGHT:
                selected = 1;
                break;
            case '\n': // Enter key
                confirmed = (selected == 0) ? 1 : 0;
                break;
        }
    }

    delwin(confirm);
    touchwin(ui->win);
    wrefresh(ui->win);
    return confirmed;
}

void display_core_stats_panel(UIWindow *ui) {
    // Initialize color pairs for progress bars
    init_pair(2, COLOR_RED, COLOR_BLACK);    // Red for high usage (>80%)
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Yellow for medium usage (>55%)
    init_pair(4, COLOR_GREEN, COLOR_BLACK);  // Green for low usage (≤55%)

    WINDOW *stats = newwin(20, 60, (ui->height - 20) / 2 + ui->starty, (ui->width - 60) / 2);
    box(stats, 0, 0);

    // Center the title and make it bold
    const char *title = "Core Statistics";
    int title_len = strlen(title);
    int title_x = (60 - title_len) / 2; // Center in a 60-char wide window
    wattron(stats, A_BOLD);
    mvwprintw(stats, 1, title_x, "%s", title);
    wattroff(stats, A_BOLD);

    // Solid separator line
    for (int i = 1; i < 60 - 1; i++) {
        mvwprintw(stats, 2, i, "_");
    }

    while (1) {
        // Clear the stats area below the title and separator
        for (int y = 3; y < 20; y++) {
            for (int x = 1; x < 59; x++) {
                mvwprintw(stats, y, x, " ");
            }
        }

        FILE *stat_file = fopen("/proc/stat", "r");
        if (stat_file) {
            char line[256];
            int core_num = 0;
            while (fgets(line, sizeof(line), stat_file)) {
                if (strncmp(line, "cpu", 3) == 0 && isdigit(line[3])) {
                    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
                    sscanf(line, "cpu%d %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", 
                           &core_num, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
                    unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
                    unsigned long work = total - idle; // Non-idle time
                    double usage = (double)(work * 100) / total;

                    // Display core usage percentage
                    mvwprintw(stats, core_num + 3, 1, "Core %d: %.2f%%", core_num, usage);

                    // Draw progress bar
                    int bar_width = 40; // Total width of the progress bar
                    int filled = (int)(usage * bar_width / 100.0); // Number of filled positions
                    int y = core_num + 3;
                    int x = 15 == 0;
                    // Select color based on usage
                    int color_pair;
                    if (usage > 80.0) {
                        color_pair = 2; // Red for high usage
                    } else if (usage > 55.0) {
                        color_pair = 3; // Yellow for medium usage
                    } else {
                        color_pair = 4; // Green for low usage
                    }

                    wattron(stats, COLOR_PAIR(color_pair));
                    for (int i = 0; i < bar_width; i++) {
                        if (i < filled) {
                            mvwprintw(stats, y, x + i, "|");
                        } else {
                            mvwprintw(stats, y, x + i, "-");
                        }
                    }
                    wattroff(stats, COLOR_PAIR(color_pair));
                }
            }
            fclose(stat_file);
        }

        wrefresh(stats);

        // Check for key press to exit (non-blocking with 1-second timeout)
        timeout(1000); // 1-second timeout
        int ch = getch();
        if (ch != ERR) {
            break; // Exit on any key press
        }
    }

    timeout(100); // Restore non-blocking mode
    delwin(stats);
    touchwin(ui->win);
    wrefresh(ui->win);
}