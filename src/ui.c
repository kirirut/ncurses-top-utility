#include "ui.h"
#include "process.h"
#include <time.h>
#include <sys/sysinfo.h>

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

void update_top_panel(UIWindow *ui) {
    WINDOW *top = newwin(1, COLS, 0, 0);
    wattron(top, COLOR_PAIR(1));

    // Memory usage from /proc/meminfo
    FILE *meminfo = fopen("/proc/meminfo", "r");
    unsigned long mem_total = 0, mem_available = 0;
    if (meminfo) {
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            if (strncmp(line, "MemTotal:", 9) == 0) {
                sscanf(line, "MemTotal: %lu kB", &mem_total);
            } else if (strncmp(line, "MemAvailable:", 13) == 0) {
                sscanf(line, "MemAvailable: %lu kB", &mem_available);
            }
        }
        fclose(meminfo);
    }
    double mem_used = (mem_total - mem_available) / 1024.0 / 1024.0; // GB
    double mem_total_gb = mem_total / 1024.0 / 1024.0; // GB

    // Load average and tasks from /proc/loadavg
    FILE *loadavg = fopen("/proc/loadavg", "r");
    double load1 = 0, load5 = 0, load15 = 0;
    int tasks_running = 0, tasks_total = 0;
    if (loadavg) {
        char line[256];
        if (fgets(line, sizeof(line), loadavg)) {
            sscanf(line, "%lf %lf %lf %d/%d", &load1, &load5, &load15, &tasks_running, &tasks_total);
        }
        fclose(loadavg);
    }

    // Uptime from sysinfo
    struct sysinfo info;
    sysinfo(&info);
    int uptime = info.uptime;
    int hours = uptime / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;

    // Get total threads (approximation via process count for simplicity)
    size_t total_threads = 0;
    DIR *dir = opendir("/proc");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (isdigit(entry->d_name[0])) {
                char path[256];
                snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
                FILE *stat_file = fopen(path, "r");
                if (stat_file) {
                    char line[256];
                    if (fgets(line, sizeof(line), stat_file)) {
                        int num_threads;
                        sscanf(line, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*d %*d %*d %*d %d", &num_threads);
                        total_threads += num_threads;
                    }
                    fclose(stat_file);
                }
            }
        }
        closedir(dir);
    }

    mvwprintw(top, 0, 0, "Mem: %.2fG/%.2fG | Tasks: %d, %lu thr, 1 running | Load average: %.2f %.2f %.2f | Uptime: %02d:%02d:%02d",
              mem_used, mem_total_gb, tasks_total, total_threads, load1, load5, load15, hours, minutes, seconds);

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
    mvwprintw(help, 9, 1, "F9: Kill Process");
    mvwprintw(help, 10, 1, "q: Quit");
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

void handle_input(UIWindow *ui, process *p_list, size_t count) {
    int ch;
    SortCriterion criterion = SORT_BY_PID;
    time_t last_refresh = time(NULL);

    while (1) {
        time_t current_time = time(NULL);
        if (difftime(current_time, last_refresh) >= 3) {
            if (parse_processes(p_list, count, criterion)) {
                display_processes(ui, p_list, count, criterion);
            }
            update_top_panel(ui);
            last_refresh = current_time;
        }

        ch = getch();
        if (ch == ERR) continue; // No input, continue loop

        if (ch == 'q') break;

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
            case KEY_F(9):
                if (ui->selected_process >= 0 && ui->selected_process < (int)count) {
                    if (display_confirmation_panel(ui, p_list[ui->selected_process].pid, p_list[ui->selected_process].name)) {
                        if (kill_process(p_list[ui->selected_process].pid) == 0) {
                            if (parse_processes(p_list, count, criterion)) {
                                display_processes(ui, p_list, count, criterion);
                            }
                            update_top_panel(ui);
                        }
                    }
                }
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