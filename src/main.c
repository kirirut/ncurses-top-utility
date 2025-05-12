#include "process.h"
#include "ui.h"

int main() {
    size_t count = get_process_count();
    process *p_list = allocate_processes(count);
    if (!p_list) return 1;

    if (!parse_processes(p_list, count, SORT_BY_PID)) {
        free_processes(&p_list, count);
        return 1;
    }

    UIWindow ui;
    init_ui(&ui);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    update_top_panel();
    display_processes(&ui, p_list, count, SORT_BY_PID);
    handle_input(&ui, p_list, count);

    cleanup_ui(&ui);
    free_processes(&p_list, count);
    return 0;
}