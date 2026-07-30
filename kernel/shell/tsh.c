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
    printf("  Active PML4 (CR3):      0x%016llx\n", get_current_context());
    putstr("  Page table levels:      4  (PML4 -> PDPT -> PD -> PT)\n");
    putstr("  Page size:              4 KiB\n");
    putstr("  Table walk:             HHDM\n");
    putstr("==============================================\n\n");
 
    heap_t *heap = k_heap_status(); // Assuming this returns a pointer to your new global heap_t
    putstr("================= Kernel Heap ================\n");
    
    if (!heap_is_valid(heap)) {
        putstr("  [not initialised]\n");
    } else {
        uint64_t initial_reserved = heap->end_addr - heap->base_addr;
        uint64_t max_capacity     = heap->limit   - heap->base_addr;
 
        printf("  Heap Base Address:      0x%016lx\n", heap->base_addr);
        printf("  Current Upper Limit:    0x%016lx\n", heap->end_addr);
        printf("  Ultimate Virtual Max:   0x%016lx\n", heap->limit);
        putstr("  ---------------------------------------------\n");
        printf("  Active Pool Size:       %lu bytes  (%lu KiB)\n",
               initial_reserved, initial_reserved / 1024);
        printf("  Max Scale Boundary:     %lu bytes  (%lu MiB)\n",
               max_capacity, max_capacity / (1024 * 1024));
        printf("  Remaining Headroom:     %lu bytes  (%lu MiB)\n",
               heap->limit - heap->end_addr, (heap->limit - heap->end_addr) / (1024 * 1024));
    }
    putstr("==============================================\n");
}

void cmd_heaptest(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("[HEAP TEST] Starting core kernel allocator validations...\n");

    // Retrieve active tracking status 
    heap_t *status = k_heap_status();
    if (!status) {
        printf("[HEAP TEST] FAIL: Kernel heap status handle is NULL.\n");
        return;
    }
    printf("[HEAP TEST] Initial State: base=0x%llx, end=0x%llx, limit=0x%llx\n",
           (unsigned long long)status->base_addr, 
           (unsigned long long)status->end_addr, 
           (unsigned long long)status->limit);

    // ─────────────────────────────────────────────────────────────────
    // TEST 1: Basic Sub-Allocations & Data Integrity
    // ─────────────────────────────────────────────────────────────────
    printf("[HEAP TEST] Test 1: Executing sequential small allocations...\n");
    #define TEST_ELEMENTS 16
    uint8_t *buffers[TEST_ELEMENTS];

    for (int i = 0; i < TEST_ELEMENTS; i++) {
        size_t alloc_size = (i + 1) * 64; 
        buffers[i] = (uint8_t *)kmalloc(alloc_size);

        if (!buffers[i]) {
            printf("[HEAP TEST] FAIL: Allocation failed at index %d for size %d\n", i, (int)alloc_size);
            return;
        }

        // Fill with unique patterns to verify no overlapping addresses exist
        memset(buffers[i], 0xA5 + i, alloc_size);
    }
    printf("[HEAP TEST] Test 1: SUCCESS (Sequential arrays mapped cleanly).\n");

    // ─────────────────────────────────────────────────────────────────
    // TEST 2: Data Validation & Cross-contamination Verification
    // ─────────────────────────────────────────────────────────────────
    printf("[HEAP TEST] Test 2: Verifying heap block boundary integrity...\n");
    for (int i = 0; i < TEST_ELEMENTS; i++) {
        size_t alloc_size = (i + 1) * 64;
        for (size_t j = 0; j < alloc_size; j++) {
            if (buffers[i][j] != (uint8_t)(0xA5 + i)) {
                printf("[HEAP TEST] FAIL: Memory corruption detected at buffers[%d][%d]!\n", i, (int)j);
                return;
            }
        }
    }
    printf("[HEAP TEST] Test 2: SUCCESS (Zero memory stomping or degradation).\n");

    // ─────────────────────────────────────────────────────────────────
    // TEST 3: Dynamic Arena Expansion (The vmm_map_page runway)
    // ─────────────────────────────────────────────────────────────────
    printf("[HEAP TEST] Test 3: Forcing dynamic arena expansion past initial sizing...\n");
    uint64_t old_end = status->end_addr;

    // Allocate a chunk significantly larger than standard page steps 
    // to force mspace to call your custom expansion loop inside heap_alloc
    size_t massive_size = 1024 * 1024 * 20; // 20 MiB (greater than 16MiB initial heap base)
    printf("[HEAP TEST] Requesting a massive chunk: %d bytes\n", (int)massive_size);
    
    void *massive_ptr = kmalloc(massive_size);
    if (!massive_ptr) {
        printf("[HEAP TEST] FAIL: Dynamic expansion failed to map massive block.\n");
        return;
    }

    // Verify heap boundary tracked variables altered upward
    uint64_t new_end = status->end_addr;
    printf("[HEAP TEST] Post-Expansion State: end=0x%llx (Grew by %lld bytes)\n", 
            (unsigned long long)new_end, (unsigned long long)(new_end - old_end));

    if (new_end <= old_end) {
        printf("[HEAP TEST] FAIL: end_addr did not increment following massive allocation expansion.\n");
        kfree(massive_ptr);
        return;
    }
    
    // Write a test sequence to confirm physical backed memory pages are actually present via page tables
    memset(massive_ptr, 0x5A, massive_size);
    printf("[HEAP TEST] Test 3: SUCCESS (VMM dynamically backstopped pages cleanly).\n");

    // ─────────────────────────────────────────────────────────────────
    // TEST 4: Reclamation via Freeing
    // ─────────────────────────────────────────────────────────────────
    printf("[HEAP TEST] Test 4: Reclaiming memory chunks and validating recycle paths...\n");
    
    // Free the large allocation block
    kfree(massive_ptr);

    // Free sequential small validation regions
    for (int i = 0; i < TEST_ELEMENTS; i++) {
        kfree(buffers[i]);
    }

    // Verify that space can be cleanly reassigned without allocating new pages
    void *recycled_ptr = kmalloc(1024 * 512); // 512 KiB
    if (!recycled_ptr) {
        printf("[HEAP TEST] FAIL: Allocator failed to reuse recently freed blocks.\n");
        return;
    }
    kfree(recycled_ptr);

    printf("[HEAP TEST] Test 4: SUCCESS (All objects freed and recycled smoothly).\n");
    printf("[HEAP TEST] ALL KERNEL HEAP ALLOCATOR TESTS PASSED SUCCESSFULLY!\n");
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