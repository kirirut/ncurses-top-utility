#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

typedef enum {
STATE_RUNNING,
STATE_SLEEPING,
STATE_DISK_SLEEP,
STATE_STOPPED,
STATE_ZOMBIE
} process_state;

typedef struct {
int pid;
char name[256];
process_state state;
uint64_t memory;
double cpu_usage;

} process;

process_state parse_process_state(char state);
const char *get_process_state_string(process_state state);
void fill_process_info(process *p_list, size_t count);
size_t get_process_count();
#endif