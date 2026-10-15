#pragma once
#include "task.h"
#include "pidTable.h"

typedef struct PACKED thread_t {
    task_t task;
    task_t* parent;
}thread_t;

typedef struct PACKED process_t {
    task_t p; //the actual process
    task_table_t* tid_table;
    thread_t* current_thread;

    

    thread_t* first_ready_thread;
    thread_t* last_ready_thread;
    thread_t* first_sleeping_thread;
    thread_t* last_sleeping_thread;
    thread_t* terminated_threads_list;

} process_t;
