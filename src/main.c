#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include "process.h"

int main() {
    initscr();  
    size_t size = get_process_count();
    process *p_list = allocate_processes(size);
    if (!p_list) {
        endwin(); 
        fprintf(stderr, "Не удалось выделить память\n");
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < size; i++) {
        printf("PID: %d | Name: %s | State: %c | Memory: %lu KB | CPU: %.2f s\n",
               p_list[i].pid, p_list[i].name, p_list[i].state, p_list[i].memory, p_list[i].cpu_usage);
    }
    printw("Num of process: %zu\n", size);
    refresh(); 
    getch();  
    free(p_list);
    endwin();  
    return 0;
}
