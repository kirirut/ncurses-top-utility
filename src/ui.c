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
                char path[512];
                snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
                FILE *stat_file = fopen(path, "r");
                if (stat_file) {
                    char line[256];
                    if (fgets(line, sizeof(line), stat_file)) {
                        int num_threads;
                        sscanf(line, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d", &num_threads);
                        total_threads += num_threads;
                    }
                    fclose(stat_file);
                }
            }
        }
        closedir(dir);
    }

    mvwprintw(top, 0, 0, "Mem: %.2fG/%.2fG | Tasks: %d, %lu thr, 1 run | Load: %.2f %.2f %.2f | Up: %02d:%02d:%02d",
              mem_used, mem_total_gb, tasks_total, total_threads, load1, load5, load15, hours, minutes, seconds);

    wattroff(top, COLOR_PAIR(1));
    wrefresh(top);
    delwin(top);
}

void display_processes(UIWindow *ui, process *p_list, size_t count, SortCriterion criterion) {
    werase(ui->win);
    box(ui->win, 0, 0);

    int items_per_page = ui->height - 3; // Leave space for header
    int total_pages = (count + items_per_page - 1) / items_per_page;
    int current_page = ui->scroll_offset / items_per_page;

    // Define column widths
    const int pid_width = 7;    // PID
    const int state_width = 5;  // STATE
    const int cpu_width = 6;    // %CPU
    const int mem_width = 6;    // MEM
    const int name_width = 20;  // NAME (adjust as needed)

    // Display column headers
    wattron(ui->win, A_BOLD);
    mvwprintw(ui->win, 1, 1, "%-*s %-*s %-*s %-*s %-*s",
              pid_width, "PID",
              state_width, "STATE",
              cpu_width, "%CPU",
              mem_width, "MEM",
              name_width, "NAME");
    wattroff(ui->win, A_BOLD);

    // Display processes
    for (int i = ui->scroll_offset; i < count && i < ui->scroll_offset + items_per_page; i++) {
        if (i == ui->selected_process) {
            wattron(ui->win, A_REVERSE);
        }

        // Convert memory to human-readable format (KB to MB or GB if large)
        char mem_str[32];
        if (p_list[i].memory >= 1024 * 1024) {
            snprintf(mem_str, sizeof(mem_str), "%.1fG", p_list[i].memory / (1024.0 * 1024.0));
        } else {
            snprintf(mem_str, sizeof(mem_str), "%luK", p_list[i].memory / 1024);
        }

        // Truncate name if too long to fit in the column
        char truncated_name[21];
        strncpy(truncated_name, p_list[i].name, name_width);
        truncated_name[name_width] = '\0'; // Ensure null termination

        // Display process data aligned with columns
        mvwprintw(ui->win, i - ui->scroll_offset + 2, 1,
                  "%-*d %-*c %*.1f %-*s %-*s",
                  pid_width, p_list[i].pid,
                  state_width, (char)p_list[i].state,
                  cpu_width, p_list[i].cpu_usage,
                  mem_width, mem_str,
                  name_width, truncated_name);

        wattroff(ui->win, A_REVERSE);
    }

    // Display sort and page info
    mvwprintw(ui->win, 0, 1, "Sort: %s | Page %d/%d",
              criterion == SORT_BY_PID ? "PID" : criterion == SORT_BY_NAME ? "Name" :
              criterion == SORT_BY_CPU ? "CPU" : "Mem", current_page + 1, total_pages);
    wrefresh(ui->win);
}

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
                    int x = 15; // Start position of the bar

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
            case KEY_F(2):
                display_core_stats_panel(ui);
                break;
            case KEY_F(3):
                if (ui->selected_process >= 0 && ui->selected_process < (int)count) {
                    display_process_details_panel(ui, &p_list[ui->selected_process]);
                }
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

void display_process_details_panel(UIWindow *ui, process *proc) {
    int details_width = 70;
    int details_height = 26; // Enough to display all fields
    WINDOW *details = newwin(details_height, details_width, (ui->height - details_height) / 2 + ui->starty, (ui->width - details_width) / 2);
    box(details, 0, 0);

    // Center the title and make it bold
    char title[256];
    snprintf(title, sizeof(title), "Process Details - PID %d", proc->pid);
    int title_len = strlen(title);
    int title_x = (details_width - title_len) / 2;
    wattron(details, A_BOLD);
    mvwprintw(details, 1, title_x, "%s", title);
    wattroff(details, A_BOLD);

    // Solid separator line
    for (int i = 1; i < details_width - 1; i++) {
        mvwprintw(details, 2, i, "_");
    }

    // Display all process fields
    mvwprintw(details, 3, 1, "Name: %s", proc->name);
    mvwprintw(details, 4, 1, "State: %c", (char)proc->state);
    mvwprintw(details, 5, 1, "Memory: %lu KB", proc->memory);
    mvwprintw(details, 6, 1, "CPU Usage: %.2f%%", proc->cpu_usage);
    mvwprintw(details, 7, 1, "Parent PID: %d", proc->ppid);
    mvwprintw(details, 8, 1, "Process Group: %d", proc->pgrp);
    mvwprintw(details, 9, 1, "Session: %d", proc->session);
    mvwprintw(details, 10, 1, "TTY Number: %d", proc->tty_nr);
    mvwprintw(details, 11, 1, "TPGID: %d", proc->tpgid);
    mvwprintw(details, 12, 1, "Flags: %u", proc->flags);
    mvwprintw(details, 13, 1, "Minor Faults: %lu", proc->minflt);
    mvwprintw(details, 14, 1, "Child Minor Faults: %lu", proc->cminflt);
    mvwprintw(details, 15, 1, "Major Faults: %lu", proc->majflt);
    mvwprintw(details, 16, 1, "Child Major Faults: %lu", proc->cmajflt);
    mvwprintw(details, 17, 1, "User Time: %lu", proc->utime);
    mvwprintw(details, 18, 1, "System Time: %lu", proc->stime);
    mvwprintw(details, 19, 1, "Priority: %d", proc->priority);
    mvwprintw(details, 20, 1, "Nice: %d", proc->nice);
    mvwprintw(details, 21, 1, "Number of Threads: %d", proc->num_threads);
    mvwprintw(details, 22, 1, "Start Time: %lu", proc->starttime);
    mvwprintw(details, 23, 1, "Virtual Memory Size: %lu", proc->vsize);
    mvwprintw(details, 24, 1, "RSS Limit: %lu", proc->rsslim);

    wrefresh(details);

    // Temporarily set blocking mode to wait for key press
    timeout(-1); // -1 means blocking
    getch();
    timeout(100); // Restore non-blocking mode

    delwin(details);
    touchwin(ui->win);
    wrefresh(ui->win);
}

void cleanup_ui(UIWindow *ui) {
    delwin(ui->win);
    endwin();
}