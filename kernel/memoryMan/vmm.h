#pragma once
#include "memory.h"
#include "heap.h"

extern uint32_t page_directory[1024];

void map_page(uint32_t v_addr, uint32_t p_addr, uint32_t pdt_flags);
void unmap_page(uint32_t v_addr);
void invalidate_page(uint32_t addr);
void reload_CR3(uint32_t p_pd_addr);

//PDT modularity for usermode
uint32_t* getCurrPDT();
void syncPDT();
void changePDT(uint32_t pdt);