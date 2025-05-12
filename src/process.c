#include "process.h"

size_t get_process_count() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return 0;
    }
    struct dirent *entry;
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            count++;
        }
    }

    closedir(dir);
    return count;
}

void free_processes(process **p, size_t count) {
    if (!p) return;
    for (size_t i = 0; i < count; i++) {
        free(p[i]);
    }
    free(p);
}

process *allocate_processes(size_t count) {
    process *p_list = malloc(count * sizeof(process));
    if (!p_list) {
        perror("malloc");
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        p_list[i].prev_utime = 0;
        p_list[i].prev_stime = 0;
        p_list[i].last_update = 0;
    }
    return p_list;
}

char* read_proc_stat(int pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    
    FILE *file = fopen(path, "r");
    if (!file) {
        return NULL;
    }

    char *buffer = malloc(1024);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    if (!fgets(buffer, 1024, file)) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

void parse_stat(const char *stat_data, process *proc) {
    int pid, ppid, pgrp, session, tty_nr, tpgid, priority, nice, num_threads;
    char name[256];
    char state;
    unsigned int flags;
    uint64_t minflt, cminflt, majflt, cmajflt, utime, stime, starttime, vsize, rsslim, rss;

    sscanf(stat_data, "%d %255s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %*d %*d %d %d %d %lu %lu %lu %lu",
           &pid, name, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags, &minflt, &cminflt, &majflt, &cmajflt,
           &utime, &stime, &priority, &nice, &num_threads, &starttime, &vsize, &rsslim, &rss);
    size_t len = strlen(name);
    if (name[0] == '(' && name[len - 1] == ')') {
        memmove(name, name + 1, len - 2);
        name[len - 2] = '\0';
    }

    time_t now = time(NULL);
    double elapsed = (proc->last_update == 0) ? 1.0 : difftime(now, proc->last_update);
    if (elapsed <= 0) elapsed = 1.0;

    uint64_t delta_ticks = (proc->prev_utime + proc->prev_stime == 0) ? 0 : 
                           ((utime + stime) - (proc->prev_utime + proc->prev_stime));
    double cpu_usage = (delta_ticks * 100.0) / (sysconf(_SC_CLK_TCK) * elapsed);
    if (cpu_usage < 0 || cpu_usage > 100) cpu_usage = 0;

    proc->pid = pid;
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = '\0';
    proc->state = (process_state)state;
    proc->memory = rss * sysconf(_SC_PAGESIZE) / 1024;
    proc->cpu_usage = cpu_usage;
    proc->ppid = ppid;
    proc->pgrp = pgrp;
    proc->session = session;
    proc->tty_nr = tty_nr;
    proc->tpgid = tpgid;
    proc->flags = flags;
    proc->minflt = minflt;
    proc->cminflt = cminflt;
    proc->majflt = majflt;
    proc->cmajflt = cmajflt;
    proc->utime = utime;
    proc->stime = stime;
    proc->priority = priority;
    proc->nice = nice;
    proc->num_threads = num_threads;
    proc->starttime = starttime;
    proc->vsize = vsize;
    proc->rsslim = rsslim;
    proc->prev_utime = utime;
    proc->prev_stime = stime;
    proc->last_update = now;
}

process* parse_processes(process* p_list, size_t max_size, SortCriterion criterion) {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    if (!dp) {
        return NULL;
    }

    size_t index = 0;
    while ((entry = readdir(dp)) && index < max_size) {
        if (!isdigit(entry->d_name[0])) continue;

        int pid = atoi(entry->d_name);
        char *data = read_proc_stat(pid);
        if (!data) continue;

        // Find if the process already exists in p_list
        int found = -1;
        for (size_t i = 0; i < max_size; i++) {
            if (p_list[i].pid == pid) {
                found = i;
                break;
            }
        }

        if (found == -1) {
            for (size_t i = 0; i < max_size; i++) {
                if (p_list[i].pid == 0) {
                    found = i;
                    break;
                }
            }
        }

        if (found != -1) {
            parse_stat(data, &p_list[found]);
            index++;
        }
        free(data);
    }

    if (index > 0) {
        qsort(p_list, index, sizeof(process), compare_processes(criterion));
    }

    closedir(dp);
    return p_list;
}

int (*compare_processes(SortCriterion criterion))(const void *, const void *) {
    switch (criterion) {
        case SORT_BY_PID:
            return compare_pid;
        case SORT_BY_NAME:
            return compare_name;
        case SORT_BY_CPU:
            return compare_cpu;
        case SORT_BY_MEMORY:
            return compare_memory;
        default:
            return compare_pid;
    }
}

int compare_pid(const void *a, const void *b) {
    return ((process *)a)->pid - ((process *)b)->pid;
}

int compare_name(const void *a, const void *b) {
    return strcmp(((process *)a)->name, ((process *)b)->name);
}

int compare_cpu(const void *a, const void *b) {
    return (int)(((process *)b)->cpu_usage - ((process *)a)->cpu_usage);
}

int compare_memory(const void *a, const void *b) {
    return (int)(((process *)b)->memory - ((process *)a)->memory);
}