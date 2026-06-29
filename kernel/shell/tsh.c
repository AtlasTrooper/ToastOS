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
    /* zero the buffer so stale bytes never leak */
    for (int i = 0; i < max; i++) buf[i] = 0;

    while (1) {
        char c = kb_getchar();

        switch (c) {
            case '\n':
            case '\r':
                buf[buf_index] = '\0';
                putchar('\n');
                return;

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

void cmd_darud(int argc, char **argv) {
    (void)argc; (void)argv;
    play_sandstorm();
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
    printf("  Active PML4 (CR3):      0x%016llx\n", get_pml4_phys());
    putstr("  Page table levels:      4  (PML4 -> PDPT -> PD -> PT)\n");
    putstr("  Page size:              4 KiB\n");
    putstr("  Table walk:             HHDM\n");
    putstr("==============================================\n\n");
 
    heap_t *heap = k_heap_status();
    putstr("================= Kernel Heap ================\n");
    if (!heap) {
        putstr("  [not initialised]\n");
    } else {
        uint64_t committed = heap->curr    - heap->base;
        uint64_t mapped    = heap->mapped  - heap->base;
 
        printf("  Heap base:              0x%016llx\n", heap->base);
        printf("  Current break:          0x%016llx\n", heap->curr);
        printf("  Mapped up to:           0x%016llx\n", heap->mapped);
        printf("  Committed (break):      %llu bytes  (%llu KiB)\n",
               committed, committed / 1024);
        printf("  Mapped (pages):         %llu bytes  (%llu KiB)\n",
               mapped, mapped / 1024);
        printf("  Slack (mapped-commit):  %llu bytes\n",
               heap->mapped - heap->curr);
    }
    putstr("==============================================\n");
}


/* ── cmd_heaptest ────────────────────────────────────────────────────────────
 *
 * Exercises kmalloc/kfree in a few stages so you can watch the heap grow
 * and shrink in meminfo:
 *
 *   Stage 1 – small allocs:  allocate 8 pointers of increasing size,
 *             write a canary value, read it back, free them all.
 *
 *   Stage 2 – large alloc:  one 64 KiB block, fill with 0xAB, verify,
 *             free it.
 *
 *   Stage 3 – realloc:  alloc 64 bytes, realloc to 256, check the old
 *             data survived.
 *
 *   Stage 4 – calloc:  make sure the returned memory is actually zeroed.
 */
void cmd_heaptest(int argc, char **argv) {
    (void)argc; (void)argv;
 
    putstr("\n[heaptest] starting...\n");
 
    /* ── Stage 1: small allocs ── */
    putstr("[heaptest] stage 1: small allocs\n");
    void *ptrs[8];
    int   failed = 0;
 
    for (int i = 0; i < 8; i++) {
        size_t sz = (size_t)(16 << i);   /* 16, 32, 64, … 2048 bytes */
        ptrs[i] = kmalloc(sz);
 
        if (!ptrs[i]) {
            printf("  [FAIL] kmalloc(%llu) returned NULL\n", (uint64_t)sz);
            failed++;
            continue;
        }
 
        /* Write a canary: every byte = low 8 bits of index */
        uint8_t *p = (uint8_t *)ptrs[i];
        for (size_t b = 0; b < sz; b++) p[b] = (uint8_t)i;
 
        /* Verify */
        int corrupt = 0;
        for (size_t b = 0; b < sz; b++)
            if (p[b] != (uint8_t)i) { corrupt = 1; break; }
 
        if (corrupt) {
            printf("  [FAIL] canary mismatch at ptrs[%d] (sz=%llu)\n",
                   i, (uint64_t)sz);
            failed++;
        } else {
            printf("  [ok]   kmalloc(%llu) -> 0x%llx\n",
                   (uint64_t)sz, (uint64_t)ptrs[i]);
        }
    }
 
    for (int i = 0; i < 8; i++)
        if (ptrs[i]) kfree(ptrs[i]);
 
    putstr(failed ? "[heaptest] stage 1: FAILED\n"
                  : "[heaptest] stage 1: passed\n");
 
    /* ── Stage 2: large alloc ── */
    putstr("[heaptest] stage 2: large alloc (64 KiB)\n");
    size_t   large_sz = 64 * 1024;
    uint8_t *large    = (uint8_t *)kmalloc(large_sz);
 
    if (!large) {
        putstr("  [FAIL] kmalloc(64K) returned NULL\n");
    } else {
        for (size_t i = 0; i < large_sz; i++) large[i] = 0xAB;
 
        int ok = 1;
        for (size_t i = 0; i < large_sz; i++)
            if (large[i] != 0xAB) { ok = 0; break; }
 
        printf("  [%s]  64 KiB fill+verify\n", ok ? "ok  " : "FAIL");
        kfree(large);
    }
 
    /* ── Stage 3: realloc ── */
    putstr("[heaptest] stage 3: realloc\n");
    uint8_t *r = (uint8_t *)kmalloc(64);
    if (!r) {
        putstr("  [FAIL] initial kmalloc for realloc returned NULL\n");
    } else {
        for (int i = 0; i < 64; i++) r[i] = (uint8_t)i;
 
        r = (uint8_t *)krealloc(r, 256);
        if (!r) {
            putstr("  [FAIL] krealloc returned NULL\n");
        } else {
            int ok = 1;
            for (int i = 0; i < 64; i++)
                if (r[i] != (uint8_t)i) { ok = 0; break; }
 
            printf("  [%s]  data survived realloc to 256 bytes\n",
                   ok ? "ok  " : "FAIL");
            kfree(r);
        }
    }
 
    /* ── Stage 4: calloc zeroing ── */
    putstr("[heaptest] stage 4: calloc zeroing\n");
    uint8_t *z = (uint8_t *)kcalloc(128, 1);
    if (!z) {
        putstr("  [FAIL] kcalloc returned NULL\n");
    } else {
        int ok = 1;
        for (int i = 0; i < 128; i++)
            if (z[i] != 0) { ok = 0; break; }
 
        printf("  [%s]  128 bytes zeroed by calloc\n", ok ? "ok  " : "FAIL");
        kfree(z);
    }
 
    putstr("[heaptest] done.\n\n");
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