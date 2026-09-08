#include "task_shell.h"
#include "../multitasking/pidTable.h"
#include "../stdlib/stdio.h"

static void print_task_row(task_t* task, void* ctx) {
    (void)ctx;
    printf("%lu\t%s\t%s\t%lu\n",
           task->pid, task->name,
           task_state_to_str(task->state),
           task->time_elapsed);
}

void shell_cmd_ps(void) {
    printf("PID\tNAME\tSTATE\tTIME_ELAPSED(ns)\n");
    pid_table_for_each(print_task_row, NULL);
}

int shell_cmd_kill(uint64_t pid) {
    int result = kill_task_by_pid(pid);
    if (result == 0) {
        printf("task %lu terminated\n", pid);
    } else {
        printf("kill failed: no such task, or task not killable (pid %lu)\n", pid);
    }
    return result;
}

void shell_cmd_status(uint64_t pid) {
    task_t* task = pid_table_lookup(pid);
    if (task == NULL) {
        printf("no such task: %lu\n", pid);
        return;
    }
    printf("pid=%lu name=%s state=%s time_elapsed=%lu ns\n",
           task->pid, task->name,
           task_state_to_str(task->state),
           task->time_elapsed);
}