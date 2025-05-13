#include "input_handler.h"
#include "top_panel.h"
#include <time.h>
#include <stdlib.h>

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