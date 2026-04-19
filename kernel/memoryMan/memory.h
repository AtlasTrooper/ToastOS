#include "../qol.h"
#include "multiboot.h"
#include "../stdlib/stdio.h"

void _kernel_v_start(void);
void _kernel_p_start(void);
void _kernel_v_end(void);
void _kernel_p_end(void);


void initMem(multiboot_info* boot_data);
