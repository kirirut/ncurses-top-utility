#include "process_display.h"
#include <string.h>
#include <stdio.h>

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