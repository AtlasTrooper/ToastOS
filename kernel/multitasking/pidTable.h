#pragma once
#include "task.h"
#include "taskTable.h"

void pid_table_init(void);

int pid_table_insert(task_t* task);
int pid_table_remove(uint64_t pid);
task_t* pid_table_lookup(uint64_t pid);

void pid_table_for_each(void (*callback)(task_t* task, void* ctx), void* ctx);

const char* task_state_to_str(task_state_t state);

uint64_t pid_alloc(void);

void pid_release(uint64_t pid);