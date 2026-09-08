#include "taskTable.h"
#include "task.h"
#include "../mmu/heap.h"

#define INIT_CAP 4

task_table_t* task_table_init(uint64_t init_cap) {
    init_cap = (init_cap == 0) ? INIT_CAP : init_cap;

    task_table_t* task_tbl = kmalloc(sizeof(task_table_t));
    if (!task_tbl) return NULL;

    task_tbl->slots = (void**)kmalloc(sizeof(void*) * init_cap);
    if (!task_tbl->slots) { kfree(task_tbl); return NULL; } //was reading a nonexistent `table` var here

    for (uint64_t i = 0; i < init_cap; i++) { task_tbl->slots[i] = NULL; }
    task_tbl->cap = init_cap; //was assigning from a nonexistent `init` var here
    return task_tbl;
}

void task_table_destroy(task_table_t* table) {
    if (!table) return;
    kfree(table->slots);
    kfree(table);
}

static int task_table_realloc(task_table_t* table, uint64_t min_cap) {
    uint64_t new_cap = table->cap * 2;
    if (new_cap < min_cap) new_cap = min_cap;

    void** new_slots = (void**)kmalloc(sizeof(void*) * new_cap);
    if (!new_slots) return -1;

    uint64_t i = 0;
    for (; i < table->cap; i++) { new_slots[i] = table->slots[i]; }
    for (; i < new_cap; i++) { new_slots[i] = NULL; }

    kfree(table->slots);
    table->slots = new_slots;
    table->cap = new_cap;
    return 0;
}

int task_table_set(task_table_t* table, uint64_t tid, void* value) {
    if (!table) return -1;
    lock_schedule();

    if (tid >= table->cap && task_table_realloc(table, tid + 1) != 0) {
        unlock_schedule();
        return -1;
    }

    if (table->slots[tid] != NULL) { unlock_schedule(); return -1; }

    table->slots[tid] = value;
    unlock_schedule();

    return 0;
}

void* task_table_get(task_table_t* table, uint64_t tid) {
    if (!table) return NULL;
    void* res = NULL;
    lock_schedule();

    if (tid < table->cap) {
        res = table->slots[tid];
    }

    unlock_schedule();
    return res;
}

int task_table_clear(task_table_t* table, uint64_t tid) {
    if (!table) return -1;
    int res = -1;
    lock_schedule();

    if (tid < table->cap) {
        table->slots[tid] = NULL;
        res = 0;
    }

    unlock_schedule();
    return res;
}

void task_table_foreach(task_table_t* table, void (*fnc_ptr)(uint64_t tid, void* value, void* ctx), void* ctx) {
    if (!table) return; //was `return NULL;` in a void function

    lock_schedule();
    for (uint64_t i = 0; i < table->cap; i++) {
        if (table->slots[i]) {
            fnc_ptr(i, table->slots[i], ctx); //was passing table->slots (the array) instead of table->slots[i]
        }
    }
    unlock_schedule();
}