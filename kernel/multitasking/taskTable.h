#pragma once
#include "../util.h"

typedef struct task_table_t {
    void** slots;
    uint64_t cap;
} task_table_t;

task_table_t* task_table_init(uint64_t initial_capacity);

/*
Frees the backing array and the table itself. Does NOT free whatever
values were stored in it - that's still on the caller.
*/
void task_table_destroy(task_table_t* table);

int task_table_set(task_table_t* table, uint64_t tid, void* value);

void* task_table_get(task_table_t* table, uint64_t tid);

int task_table_clear(task_table_t* table, uint64_t tid);

void task_table_foreach(task_table_t* table, void (*fnc_ptr)(uint64_t tid, void* value, void* ctx), void* ctx);