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