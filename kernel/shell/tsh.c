#include "tsh.h"

#define ARGC_MAX     16
#define HISTORY_MAX   8
#define KB_BUF_SIZE 256

static int  history_count      = 0;
static int  prev_command_index = 0;
static char cmd_history[HISTORY_MAX][KB_BUF_SIZE];
static char buf[KB_BUF_SIZE];
static int  buf_index = 0;

void init_shell(void) {
    print_banner();
    cmd_fetch(0, NULL);
    while (1) {
        print_prompt();
        readline(buf, KB_BUF_SIZE);
        if (!buf[0]) continue;

        update_history(buf);
        shell_exec(buf);
        yield();
    }
}

void readline(char *buf, int max) {
    buf_index = 0;
    for (int i = 0; i < max; i++) buf[i] = 0;

    while (1) {
        char c = kb_getchar();

        switch (c) {
            case '\n':
            case '\r':
                buf[buf_index] = '\0';
                putchar('\n');
                return;
            case '\t':
                for(uint64_t i = 0; i < 4; i++) {
                    if (buf_index < max - 1){
                        buf[buf_index++] = ' ';
                        putchar(' ');
                    }
                }
                continue;
            case '\b':
                if (buf_index > 0) {
                    buf_index--;
                    buf[buf_index] = '\0';
                    putchar('\b');
                }
                continue;
            
            case '\x1b':
                if (kb_getchar() == '[') {
                    char dir = kb_getchar();
                    if      (dir == 'A') get_prev_cmd();
                    else if (dir == 'B') get_next_cmd();
                }
                continue;
            
            default:
                break;
        }

        if (buf_index < max - 1) {
            buf[buf_index++] = c;
            putchar(c);
        }
    }
}

int parse_args(char *line, char **argv, int max_args) {
    int argc = 0;

    while (line != NULL) {
        if (*line == ' ') { line++; continue; }
        if (!*line) break;

        argv[argc++] = line;
        while (*line && *line != ' ') line++;
        if (*line) *line++ = '\0';
        if (argc == max_args) break;
    }
    return argc;
}

void shell_exec(char *line) {
    char *argv[ARGC_MAX];
    int   argc = parse_args(line, argv, ARGC_MAX);
    if (argc == 0) return;

    int cmd_count = (int)(sizeof(commands) / sizeof(command_t));
    for (int i = 0; i < cmd_count; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].ptr(argc, argv);
            return;
        }
    }

    printf("tsh: unknown command: %s\n", argv[0]);
}

void shell_clear(void) {
    for (int i = 0; i < KB_BUF_SIZE; i++) buf[i] = 0;
    terminal_clear();
    print_prompt();
}

void update_history(char *line) {
    if (!line[0]) return;
    strcpy(cmd_history[(history_count++) % HISTORY_MAX], line);
    prev_command_index = history_count % HISTORY_MAX;
    if ((history_count % HISTORY_MAX) == 0) history_count = 0;
}

void get_prev_cmd(void) {
    for (int i = 0; i < KB_BUF_SIZE; i++) buf[i] = 0;
    clearCurrentLine();
    print_prompt();

    int prev_idx = (prev_command_index - 1 + HISTORY_MAX) % HISTORY_MAX;
    if (strlen(cmd_history[prev_idx]) >= 1)
        prev_command_index = prev_idx;

    strcpy(buf, cmd_history[prev_command_index]);
    putstr(buf);
    buf_index = (int)strlen(buf);
}

void get_next_cmd(void) {
    for (int i = 0; i < KB_BUF_SIZE; i++) buf[i] = 0;
    clearCurrentLine();
    print_prompt();

    int next_idx = (prev_command_index + 1) % HISTORY_MAX;
    if (strlen(cmd_history[next_idx]) >= 1) {
        prev_command_index = next_idx;
        strcpy(buf, cmd_history[prev_command_index]);
        putstr(buf);
    }

    buf_index = (int)strlen(buf);
}

void cmd_help(int argc, char **argv) {
    int cmd_count = (int)(sizeof(commands) / sizeof(command_t));

    if (argc < 2) {
        putstr("No command given - printing all commands\n\n");
        putstr("            CMD LIST            \n");
        putstr("--------------------------------\n");
        for (int i = 0; i < cmd_count; i++) {
            if (strlen(commands[i].help_msg) > 0)
                printf("  %s : %s \n", commands[i].name, commands[i].help_msg);
        }
        putstr("--------------------------------\n");
    } else {
        int found = 0;
        for (int i = 0; i < cmd_count; i++) {
            if (strcmp(argv[1], commands[i].name) == 0) {
                printf("%s: %s\n", commands[i].name, commands[i].help_msg);
                found = 1;
            }
        }
        if (!found) printf("help: unknown command '%s'\n", argv[1]);
    }
}

void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        putstr(argv[i]);
        if (i < argc - 1) putchar(' ');
    }
    putchar('\n');
}

void cmd_lsh(int argc, char **argv) {
    (void)argc; (void)argv;
    int count = history_count % HISTORY_MAX;
    if (count == 0 && history_count > 0) count = HISTORY_MAX;
    for (int i = 0; i < count; i++)
        printf("  %d : %s\n", i, cmd_history[i]);
}

void cmd_meminfo(int argc, char **argv) {
    (void)argc; (void)argv;
putstr("===============\n");
putstr("= ToastOS MMU =\n");
putstr("===============\n\n");
const pmm_header_t *pmm = get_pmm_header();
if (!pmm) {
putstr("[PMM] Error: header is NULL\n");
return;
}
uint64_t total_mib = (pmm->max_frames   * PAGE_SIZE) / (1024 * 1024);
uint64_t free_mib  = (pmm->free_frames  * PAGE_SIZE) / (1024 * 1024);
uint64_t used_mib  = total_mib - free_mib;
putstr("==================== PMM ====================\n");
printf("  HHDM base:              0x%016llx\n", pmm->hhdm_base);
printf("  Kernel phys base:       0x%016llx\n", pmm->kernel_phys_base);
printf("  Kernel virt base:       0x%016llx\n", pmm->kernel_virt_base);
printf("  Kernel phys end:        0x%016llx\n", pmm->kernel_phys_end);
putstr("  ---------------------------------------------\n");
printf("  Bitmap phys addr:       0x%016llx\n", pmm->bitmap_phys);
printf("  Bitmap virt ptr:        0x%016llx\n", (uint64_t)pmm->b_map);
printf("  Bitmap size:            %lu Bytes\n",  pmm->bitmap_bytes);
putstr("  ---------------------------------------------\n");
printf("  First alloc frame:      0x%016llx\n",   pmm->alloc_start_frame);
printf("  Total frames:           %llu\n",        pmm->max_frames);
printf("  Free frames:            %llu\n",        pmm->free_frames);
printf("  Memory:                 %llu MiB total  %llu MiB used  %llu MiB free\n",
           total_mib, used_mib, free_mib);
putstr("==============================================\n\n");
putstr("==================== VMM ====================\n");
printf("  Active PML4 (CR3):      0x%016llx\n", get_current_context()->pml_phys);
putstr("  Page table levels:      4  (PML4 -> PDPT -> PD -> PT)\n");
putstr("  Page size:              4 KiB\n");
putstr("  Table walk:             HHDM\n");
putstr("==============================================\n\n");
heap_t *heap = k_heap_status();
putstr("================= Kernel Heap (sbrk) ================\n");
if (!heap_is_valid(heap)) {
putstr("  [not initialised]\n");
} else {
uint64_t used_virtual = heap->brk - heap->base_addr;
uint64_t backed_bytes = heap->mapped_end - heap->base_addr;
uint64_t max_capacity = heap->limit - heap->base_addr;
uint64_t headroom     = heap->limit - heap->brk;

printf("  Heap Base Address:      0x%016lx\n", heap->base_addr);
printf("  Current Break (brk):    0x%016lx\n", heap->brk);
printf("  Mapped/Backed Up To:    0x%016lx\n", heap->mapped_end);
printf("  Ultimate Virtual Max:   0x%016lx\n", heap->limit);
putstr("  ---------------------------------------------\n");
printf("  Logical Used Size:      %lu bytes  (%lu KiB)\n",
               used_virtual, used_virtual / 1024);
printf("  Physically Backed:      %lu bytes  (%lu KiB)\n",
               backed_bytes, backed_bytes / 1024);
printf("  Max Scale Boundary:     %lu bytes  (%lu MiB)\n",
               max_capacity, max_capacity / (1024 * 1024));
printf("  Remaining Headroom:     %lu bytes  (%lu MiB)\n",
               headroom, headroom / (1024 * 1024));
}
putstr("=======================================================\n");
}

void cmd_pmmtest(int argc, char **argv) {
    debug_print("[TEST] Starting PMM tests...\n");

    uint64_t frame1 = pmm_alloc();
    if (!frame1 || (frame1 & 0xFFF) != 0) {
        debug_print("[FAIL] PMM allocation failed or unaligned!\n");
        for(;;);
    }

    uint64_t frame2 = pmm_alloc();
    if (frame1 == frame2) {
        debug_print("[FAIL] PMM allocated duplicate frames!\n");
        for(;;);
    }

    pmm_free(frame1);
    pmm_free(frame2);
    debug_print("[PASS] PMM tests passed!\n");
}

void cmd_vmmtest(int argc, char **argv) {
    debug_print("[TEST] Starting VMM tests...\n");

    uint64_t phys = pmm_alloc();
    printf("\nPHYS %p\n", phys);
    uint64_t virt = 0xFFFF900000000000ULL;
    vmm_map_page(get_current_context(), virt, phys, VMM_FLAGS_KERNEL_RW);

    volatile uint64_t *test_ptr = (volatile uint64_t*)virt;
    *test_ptr = 0x123456789ABCDEF0ULL;

    if (*test_ptr != 0x123456789ABCDEF0ULL) {
        debug_print("[FAIL] VMM read/write validation failed!\n");
        debug_print_hex("Read value: ", *test_ptr);
        for(;;);
    }

    debug_print("[PASS] VMM mapping and round-trip memory test passed!\n");
}

void cmd_heaptest(int argc, char **argv) {
    (void)argc; (void)argv;

    debug_print("[TEST] Starting kernel heap (sbrk) tests...\n");

    heap_t *heap = k_heap_status();
    if (!heap_is_valid(heap)) {
        debug_print("[FAIL] Kernel heap not initialised!\n");
        return;
    }

    uint64_t mapped_before = heap->mapped_end;
    printf("[TEST] Initial brk:        0x%016lx\n", heap->brk);
    printf("[TEST] Initial mapped_end: 0x%016lx\n", heap->mapped_end);

    #define N_ALLOCS 64
    void *ptrs[N_ALLOCS];
    size_t alloc_size = 8192;

    for (int i = 0; i < N_ALLOCS; i++) {
        ptrs[i] = kmalloc(alloc_size);
        if (!ptrs[i]) {
            debug_print("[FAIL] kmalloc returned NULL!\n");
            for (;;);
        }
        memset(ptrs[i], 0xAA, alloc_size);
    }

    uint64_t mapped_after_alloc = heap->mapped_end;
    printf("[TEST] mapped_end after allocs: 0x%016lx\n", mapped_after_alloc);

    if (mapped_after_alloc <= mapped_before) {
        debug_print("[FAIL] Heap did not grow after allocations!\n");
        for (;;);
    }

    for (int i = 0; i < N_ALLOCS; i++) {
        uint8_t *p = (uint8_t*)ptrs[i];
        for (size_t j = 0; j < alloc_size; j++) {
            if (p[j] != 0xAA) {
                debug_print("[FAIL] Heap memory corruption detected!\n");
                for (;;);
            }
        }
    }

    for (int i = 0; i < N_ALLOCS; i++) {
        kfree(ptrs[i]);
    }

    // In case the automatic trim threshold wasn't crossed by these frees,
    // force it so the test result is deterministic.
    kheap_trim(0);

    uint64_t mapped_after_free = heap->mapped_end;
    printf("[TEST] mapped_end after free+trim: 0x%016lx\n", mapped_after_free);

    if (mapped_after_free >= mapped_after_alloc) {
        debug_print("[FAIL] Heap did not shrink after freeing memory!\n");
        for (;;);
    }

    debug_print("[PASS] Kernel heap grow/shrink test passed!\n");
}

const char *memmap_type_str(uint64_t type) {
    switch (type) {
        case 0: return "Usable";
        case 1: return "Reserved";
        case 2: return "ACPI Reclaimable";
        case 3: return "ACPI NVS";
        case 4: return "Bad Memory";
        case 5: return "Bootloader Reclaimable";
        case 6: return "Kernel and Modules";
        case 7: return "Framebuffer";
        default: return "Unknown";
    }
}
 
void cmd_memmap(int argc, char **argv) {

    (void)argc; (void)argv;
 
    const pmm_header_t *pmm = get_pmm_header();
    if (!pmm) { putstr("[memmap] PMM not initialised\n"); return; }
 
    struct limine_memmap_response *memmap = get_memmap();
    if (!memmap) { putstr("[memmap] no Limine memmap response\n"); return; }
 
    putstr("========================================================\n");
    putstr(" #   Base                 Length               Type\n");
    putstr("========================================================\n");
 
    uint64_t total_usable = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
 
        uint64_t mib = ent->length / (1024 * 1024);
        uint64_t kib = (ent->length % (1024 * 1024)) / 1024;
 
        printf(" [%llu] 0x%016llx   0x%016llx   %s",
               i, ent->base, ent->length, memmap_type_str(ent->type));
 
        if (mib > 0)
            printf("  (%llu MiB)\n", mib);
        else
            printf("  (%llu KiB)\n", kib);
 
        if (ent->type == LIMINE_MEMMAP_USABLE)
            total_usable += ent->length;
    }
 
    putstr("========================================================\n");
    printf(" Total usable: %llu MiB (%llu bytes)\n",
           total_usable / (1024 * 1024), total_usable);
    putstr("========================================================\n");
}

void cmd_fetch(int argc, char **argv) {
    (void)argc; 
    (void)argv;

    char cpu_name[49] = {0};
    uint32_t *brand = (uint32_t *)cpu_name;
    for (uint32_t i = 0; i < 3; i++) {
        __cpuid(0x80000002 + i, brand[i*4 + 0], brand[i*4 + 1], 
                                brand[i*4 + 2], brand[i*4 + 3]);
    }

    uint64_t uptime = get_uptime_seconds();
    uint64_t mins = uptime / 60;
    uint64_t secs = uptime % 60;

    const pmm_header_t *pmm = get_pmm_header();
    uint64_t total_mib = 0, used_mib = 0;
    
    if (pmm) {
        total_mib = (pmm->max_frames  * PAGE_SIZE) / (1024 * 1024);
        uint64_t free_mib = (pmm->free_frames * PAGE_SIZE) / (1024 * 1024);
        used_mib  = total_mib - free_mib;
    }

    putstr("\n");
    printf(" \033[36muser\033[0m@\033[36mToastOS\033[0m\n");
    printf(" -------------------\n");
    printf(" \033[33mOS\033[0m: ToastOS x86_64\n");
    printf(" \033[33mUptime\033[0m: %llu mins, %llu secs\n", mins, secs);
    printf(" \033[33mCPU\033[0m: %s\n", cpu_name[0] ? cpu_name : "Unknown Processor");
    printf(" \033[33mMemory\033[0m: %llu MiB / %llu MiB\n", used_mib, total_mib);
    printf("\n");
    
    printf(" \033[41m   \033[42m   \033[43m   \033[44m   \033[45m   \033[46m   \033[47m   \033[0m\n");
    printf("\n");

}

//TODO: note to self, add a 'reap' that frees dead tasks and make this into a proper taskinfo at some point
void cmd_poll_task_time(int argc, char **argv) {
    update_task_time();
    thread_t* tracker = get_pid0();
    printf("[TEST] Task time check:\n");
    int looped = 0;
    while (looped != 2) {
        printf("%d : %s : %llu \n", tracker->pid, tracker->name, tracker->time_elapsed);
        tracker = tracker->next;
        if(tracker->pid == 1) looped ++;
    }
}

void cmd_exit(int argc, char **argv) {
    (void)argc; (void)argv;
    putstr("Halting CPU — adios amigo!\n");
    asm volatile("cli; hlt");
}

void print_banner(void) {
    console_set_color(CON_WHITE, CON_BLACK);
    putstr("=================================================================\n");
    putstr(" ToastOS x86_64  |  tsh  |  est. 2026  Tomer Wiesel\n");
    putstr(" (eden was here :)\n");
    putstr("=================================================================\n");
    console_set_color(CON_WHITE, CON_BLACK);
}

void print_prompt(void) {
    console_set_color(CON_LIGHT_GREEN, CON_BLACK);
    putstr("tsh");
    console_set_color(CON_WHITE, CON_BLACK);
    putstr("> ");
    console_set_color(CON_WHITE, CON_BLACK);
}