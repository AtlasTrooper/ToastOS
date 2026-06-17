#pragma once
#include "../qol.h"

typedef struct process_t{
    uint32_t pid;
    uint32_t* s;
    uint32_t lim;
}process_t;