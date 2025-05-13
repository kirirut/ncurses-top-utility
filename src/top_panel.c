#include "top_panel.h"
#include <time.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

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