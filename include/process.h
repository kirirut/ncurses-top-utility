#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

typedef enum {
    PROCESS_STATE_RUNNING = 'R',
    PROCESS_STATE_SLEEPING = 'S',
    PROCESS_STATE_DISK_SLEEP = 'D',
    PROCESS_STATE_ZOMBIE = 'Z',
    PROCESS_STATE_STOPPED = 'T',
    PROCESS_STATE_TRACING_STOP = 't',
    PROCESS_STATE_WAKING = 'W',
    PROCESS_STATE_PARKED = 'P',
    PROCESS_STATE_IDLE = 'I',
    PROCESS_STATE_DEAD = 'X',
    PROCESS_STATE_UNKNOWN = '?'
} process_state;

typedef struct {
    int pid;
    char name[256];
    process_state state;
    uint64_t memory;
    double cpu_usage;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned int flags;
    uint64_t minflt;
    uint64_t cminflt;
    uint64_t majflt;
    uint64_t cmajflt;
    uint64_t utime;
    uint64_t stime;
    int priority;
    int nice;
    int num_threads;
    uint64_t starttime;
    uint64_t vsize;
    uint64_t rsslim;
} process;

size_t get_process_count();
void free_processes(process **p, size_t count);
process *allocate_processes(size_t count);
process* parse_processes(process* p_list, size_t max_size);
void parse_stat(const char *stat_data, process *proc);
char* read_proc_stat(int pid);


#endif