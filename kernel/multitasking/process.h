#pragma once
#include "../util.h"
#include "../idt/idt.h"

typedef struct process_t {
    uint64 pid;
    uint64 cr3;
    char name[32]; //arb len

    process_t *next;
} process_t;
