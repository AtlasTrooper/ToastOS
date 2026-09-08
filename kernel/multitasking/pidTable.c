#include "pidTable.h"
#include "../mmu/heap.h"

#define PID_TABLE_INITIAL_CAPACITY 64

//global pid table
static task_table_t* g_pid_table = NULL;

void pid_table_init(void) {
    g_pid_table = task_table_init(PID_TABLE_INITIAL_CAPACITY);
}

int pid_table_insert(task_t* task) {
    if (task == NULL) return -1;
    return task_table_set(g_pid_table, task->pid, task);
}

int pid_table_remove(uint64_t pid) {
    return task_table_clear(g_pid_table, pid);
}

task_t* pid_table_lookup(uint64_t pid) {
    return (task_t*)task_table_get(g_pid_table, pid);
}

typedef struct {
    void (*callback)(task_t* task, void* ctx);
    void* ctx;
} pid_for_each_shim_t;

static void pid_for_each_trampoline(uint64_t tid, void* value, void* ctx) {
    (void)tid;
    pid_for_each_shim_t* shim = (pid_for_each_shim_t*)ctx;
    shim->callback((task_t*)value, shim->ctx);
}

void pid_table_for_each(void (*callback)(task_t*, void*), void* ctx) {
    pid_for_each_shim_t shim = { callback, ctx };
    task_table_foreach(g_pid_table, pid_for_each_trampoline, &shim);
}

const char* task_state_to_str(task_state_t state) {
    switch (state) {
        case TASK_READY:            return "READY";
        case TASK_RUNNING:          return "RUNNING";
        case TASK_BLOCKED:          return "BLOCKED";
        case TASK_WAITING_FOR_LOCK: return "WAITING";
        case TASK_PAUSED:           return "PAUSED";
        case TASK_SLEEPING:         return "SLEEPING";
        case TASK_DEAD:             return "DEAD";
        default:                    return "UNKNOWN";
    }
}

typedef struct free_pid_node {
    uint64_t pid;
    struct free_pid_node* next;
} free_pid_node_t;

static free_pid_node_t* free_pid_list = NULL;
static uint64_t next_greatest_pid = 1;

uint64_t pid_alloc(void) {
    lock_schedule();

    if (free_pid_list != NULL) {
        free_pid_node_t* node = free_pid_list;
        uint64_t pid = node->pid;
        free_pid_list = node->next;
        unlock_schedule();
        kfree(node);
        return pid;
    }

    uint64_t pid = next_greatest_pid++;
    unlock_schedule();
    return pid;
}

void pid_release(uint64_t pid) {
    free_pid_node_t* node = (free_pid_node_t*)kmalloc(sizeof(free_pid_node_t));
    if (node == NULL) return; //if heap failiure ditch pid
    
    node->pid = pid;

    lock_schedule();
    node->next = free_pid_list;
    free_pid_list = node;
    unlock_schedule();
}