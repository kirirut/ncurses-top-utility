#include "graph.h"
void draw_memory_bar() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("Ошибка открытия файла /proc/meminfo");
        return;
    }

    long total_memory = 0;
    long free_memory = 0;
    long available_memory = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %ld kB", &total_memory);
        } else if (strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line, "MemFree: %ld kB", &free_memory);
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %ld kB", &available_memory);
        }

        if (total_memory && free_memory && available_memory) {
            break;
        }
    }

    fclose(file);

    // Расчет использования памяти
    long used_memory = total_memory - available_memory;
    double memory_usage = (double)used_memory / total_memory * 100;

    // Отображение полоски загрузки
    int bar_length = 40;  // Длина полоски
    int progress = (int)(memory_usage / 100 * bar_length);

    mvprintw(1, 0, "Memory Usage:[");
    for (int i = 0; i < bar_length; i++) {
        if (i < progress)
            mvaddch(1, i + 13, '#');  // Заполнение полоски символами #
        else
            mvaddch(1, i + 13, ' ');  // Пустое пространство
    }
    mvprintw(1, 55, "] %.2f%%", memory_usage);  // Отображение процента
}
void draw_disk_bar() {
    FILE *file = popen("df --output=pcent / | tail -n 1", "r");
    if (!file) {
        perror("Ошибка получения информации о диске");
        return;
    }

    int disk_usage = 0;
    fscanf(file, "%d", &disk_usage);
    fclose(file);

    // Отображение полоски загрузки
    int bar_length = 40;  // Длина полоски
    int progress = (int)(disk_usage / 100.0 * bar_length);

    mvprintw(3, 0, "Disk Usage: [");
    for (int i = 0; i < bar_length; i++) {
        if (i < progress)
            mvaddch(3, i + 13, '#');  // Заполнение полоски символами #
        else
            mvaddch(3, i + 13, ' ');  // Пустое пространство
    }
    mvprintw(3, 55, "] %d%%", disk_usage);  // Отображение процента
}