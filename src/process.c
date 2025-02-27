#include "process.h"

process_state parse_process_state(char state)
{
switch( state ) 
    {
    case 'R': return STATE_RUNNING;
    case 'S': return STATE_SLEEPING;
    case 'D': return STATE_DISK_SLEEP;
    case 'T': return STATE_STOPPED;
    case 'Z': return STATE_ZOMBIE;
    default: return STATE_SLEEPING;
    }   
}

const char *get_process_state_string(process_state state)
{
switch (state)
{
case STATE_RUNNING: return "Running";
case STATE_SLEEPING: return "Sleeping";
case STATE_DISK_SLEEP: return "Disk sleep";
case STATE_STOPPED: return "Stopped";
case STATE_ZOMBIE: return "Unknown";
}
}
 
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

// Парсинг строки stat в структуру process
void parse_stat(const char *stat_data, process *proc) {
    char buffer[1024];
    strncpy(buffer, stat_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token;
    char *saveptr;
    int index = 1;

    token = strtok_r(buffer, " ", &saveptr);
    if (!token) return;
    
    proc->pid = atoi(token);

    // Парсим имя процесса (COMM)
    token = strchr(saveptr, '(') + 1;
    char *end = strchr(token, ')');
    if (end) {
        *end = '\0';
        strncpy(proc->name, token, sizeof(proc->name) - 1);
        proc->name[sizeof(proc->name) - 1] = '\0';
        saveptr = end + 2;
    } else {
        strcpy(proc->name, "UNKNOWN");
    }

    // Парсим state
    token = strtok_r(NULL, " ", &saveptr);
    if (token) proc->state = (process_state)token[0];

    // Пропускаем 21 поле до memory (RSS, 24-е поле)
    for (int i = 4; i < 24; i++) {
        token = strtok_r(NULL, " ", &saveptr);
        if (!token) return;
    }
    proc->memory = strtoull(token, NULL, 10);

    // Парсим CPU время (utime + stime, 14-е и 15-е поля)
    uint64_t utime, stime;
    for (int i = 24; i < 14; i++) {
        token = strtok_r(NULL, " ", &saveptr);
        if (!token) return;
    }
    utime = strtoull(token, NULL, 10);
    token = strtok_r(NULL, " ", &saveptr);
    if (!token) return;
    stime = strtoull(token, NULL, 10);

    // Вычисляем примерную загрузку CPU
    proc->cpu_usage = (double)(utime + stime) / sysconf(_SC_CLK_TCK);
}

// Перебор процессов в /proc и заполнение массива структур
process* parse_processes(process* p_list, size_t max_size) {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    if (!dp) {
        perror("Ошибка открытия /proc");
        return 0;
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
void print_processes(process* p_list, size_t size) {
    printf("%-6s %-25s %-10s %-10s %-10s\n", "PID", "Name", "State", "Memory", "CPU (%)");
    printf("--------------------------------------------------------------\n");
    for (size_t i = 0; i < size; i++) {
        const char* state_str[] = {"RUNNING", "SLEEPING", "DISK_SLEEP", "STOPPED", "ZOMBIE", "UNKNOWN"};
        printf("%-6d %-25s %-10s %-10lu %-10.2f\n",
               p_list[i].pid,
               p_list[i].name,
               state_str[p_list[i].state],
               p_list[i].memory,
               p_list[i].cpu_usage);
    }
}