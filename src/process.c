#include "process.h"

 size_t get_process_count()
 {
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

    return p_list;
}
char* read_proc_stat(int pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Ошибка открытия файла");
        return NULL;
    }

    char *buffer = malloc(1024);
    if (!buffer) {
        perror("Ошибка выделения памяти");
        fclose(file);
        return NULL;
    }

    if (!fgets(buffer, 1024, file)) {
        perror("Ошибка чтения файла");
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
    
    // Убираем круглые скобки у имени
    size_t len = strlen(name);
    if (name[0] == '(' && name[len - 1] == ')') {
        memmove(name, name + 1, len - 2);
        name[len - 2] = '\0';
    }

    proc->pid = pid;
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = '\0';
    proc->state = (process_state)state;
    proc->memory = rss * sysconf(_SC_PAGESIZE);
    proc->cpu_usage = (double)(utime + stime) / sysconf(_SC_CLK_TCK);
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
}

process* parse_processes(process* p_list, size_t max_size) {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    if (!dp) {
        perror("Ошибка открытия /proc");
        return NULL;
    }

    size_t index = 0;
    while ((entry = readdir(dp)) && index < max_size) {
        if (!isdigit(entry->d_name[0])) continue;

        int pid = atoi(entry->d_name);
        char *data = read_proc_stat(pid);
        if (!data) continue;

        parse_stat(data, &p_list[index]);
        free(data);
        index++;
    }

    closedir(dp);
    return p_list;
}
