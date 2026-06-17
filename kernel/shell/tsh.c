#include "tsh.h"

/*
max size of a single command, currently hardcoded 
and stack allocated soon to be malloced and heap allocated
*/
#define ARGC_MAX 16
#define HISTORY_MAX 8

static int history_count = 0; 
static char cmd_history[HISTORY_MAX][KB_BUF_SIZE];
static char buf[KB_BUF_SIZE];
static int buf_index = 0;
static int prev_command_index = 0;
#pragma region shell loop
void init_shell() {
    // char buf[KBUF_SIZE];
    print_banner();

    while(1) {
        print_prompt();
        readline(buf, KB_BUF_SIZE);
        if(!buf[0]) {continue;}

        update_history(buf);
        shell_exec(buf);

    }
}

void readline(char *buf, int max){
    buf_index = 0;
    memset(buf, 0, max);

    while (1) {
        char c = kb_getchar();

        /*
        As this is a readline function, anything
        that would get us to leave the current line
        should get us to return.
        */
        switch(c) {
            case '\n':
            case '\r':
                buf[buf_index] == '\0';
                putchar('\n');
                return;
            case '\b':
                if (buf_index > 0) {
                    buf_index--;
                    buf[buf_index] = '\0';
                    putchar('\b');
                    //putchar(' ');
                    //putchar('\b');
                }
                continue;

            default:
                break;
        }
        if (buf_index < max -1) {
            buf[buf_index++] = c;
            putchar(c);
        }
    }

}

int parse_args(char *line, char **argv, int max_args) {
    int argc = 0;

    while(line != NULL) {
        if (*line == ' ') {line++; continue;}
        if (!*line) break;
        argv[argc++] = line;
        while (*line && *line != ' ') {line++;}
        if (*line) *line++ = '\0';
        if (argc == ARGC_MAX) break;
    }
    return argc;

}

void shell_exec(char *line) {
    char *argv[ARGC_MAX];
    int argc = parse_args(line, argv, ARGC_MAX);

    int cmd_length = sizeof(commands)/sizeof(command_t);
    for(int i = 0; i < cmd_length; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].ptr(argc, argv);
            return;
        }
    }

    printf("Sorry, bad command, try again.\n");
}

void shell_clear() {
    memset(buf, 0, KB_BUF_SIZE);
    terminal_clear();
    print_prompt();
}
#pragma endregion shel loop

#pragma region history
void update_history(char *line) {
    if(!line[0]) return;
    strcpy(cmd_history[(history_count++)%HISTORY_MAX], line);
    prev_command_index = (history_count%HISTORY_MAX);
    if((history_count%HISTORY_MAX) == 0) history_count = 0;
}

void get_prev_cmd() {
    memset(buf, 0, KB_BUF_SIZE);
    clearCurrentLine();
    print_prompt();

    int prev_idx = (prev_command_index-1 + HISTORY_MAX)%HISTORY_MAX;

    if (strlen(cmd_history[prev_idx]) >= 2) {
        prev_command_index = prev_idx;
    }

    strcpy(buf, cmd_history[prev_command_index]);
    putstr(buf);

    buf_index = strlen(buf);
}

void get_next_cmd() {
    memset(buf, 0, KB_BUF_SIZE);
    clearCurrentLine();
    print_prompt();
    
    int next_idx = (prev_command_index + 1)%HISTORY_MAX;

    if (strlen(cmd_history[next_idx]) >= 2) {
        prev_command_index = next_idx;
        strcpy(buf, cmd_history[prev_command_index]);
        putstr(buf);
    }

    buf_index = strlen(buf);
}
#pragma endregion history

#pragma region commands
void cmd_help(int argc, char **argv) {
    if (argc < 2) {
        putstr("No command given, printing all commands\n\n");
        putstr("                 CMD LIST                   ");
        putstr("\n---------------------------------------\n");
        for (int i = 0; i < sizeof(commands)/sizeof(command_t); i++) {
            if (strlen(commands[i].help_msg) > 1) {
                printf("%s : %s \n", commands[i].name, commands[i].help_msg);
            }
        }
        putstr("---------------------------------------\n");
    } else {
        for (int i = 0; i < sizeof(commands)/sizeof(command_t); i++) {
            if (strcmp(argv[1], commands[i].name) == 0) {
                printf("%s \n", commands[i].help_msg);
            }
        }
    }
}

void cmd_darud(int argc, char **argv) {
    play_sandstorm();
}

void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        putstr(argv[i]);
        if (i < argc-1) putchar(' ');
    }
    putchar('\n');
}

void cmd_lsh(int argc, char **argv) {
    for(int i = 0; i < (history_count%HISTORY_MAX); i++) {
        printf("%d : %s \n", i, cmd_history[i]);
    }
}

void cmd_memstat(int argc, char **argv) {
    putstr("[MEMORY STATUS]\n\n");
    heap_t* k_heap = k_heap_status();
    if(!k_heap) {
        putstr("The heap does not yet exist or is not working\n");
    } else {
        printf("HEAP BASE: %p\nHEAP MAX: %p\n\n", k_heap->s, k_heap->max);
    }

    printf("TOTAL SIZE OF KERNEL_HEAP: %u bytes\n", (uint32_t)(k_heap->max-k_heap->s));
}

/*
Once the OS becomes more advanced, this will be phased
out for a function that prunes everything while also saving state.
*/
void cmd_exit(int argc, char **argv) {
    putstr("Halting CPU, adios amigo!\n");
    asm volatile("cli; hlt");
}

#pragma endregion commands

#pragma region misc
void print_banner(void) {
    terminal_set_color(vga_entry_color(VGA_LIGHT_BROWN, VGA_BLACK));
    putstr("=================================================================\n");
    putstr("Welcome to the Toast Operating System| est. 2026 Tomer Wiesel    \n(eden was here :)\n");
    putstr("=================================================================\n");
    terminal_set_color(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}
 
void print_prompt(void) {
    terminal_set_color(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    putstr("tsh");
    terminal_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
    putstr("> ");
    terminal_set_color(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

void test_malloc_basic(int argc, char **argv){
    heap_t *h = k_heap_status();
    if(h == NULL){
        putstr("FAIL: heap not initialized\n");
        return;
    }

    uint32_t size_before = h->max - h->s;
    printf("heap size before: %x\n", size_before);

    void *a = malloc(16);

    uint32_t size_after_a = h->max - h->s;
    printf("heap size after a: %x\n", size_after_a);

    // morebytes rounds up to MIN_ALLOC units, which heapafus rounds up
    // to whole pages -- first malloc should grow the heap.
    if(size_after_a <= size_before){
        putstr("FAIL: heap did not grow on first malloc\n");
    }
    if(size_after_a % 4096 != 0){
        putstr("FAIL: heap size not page-aligned\n");
    }

    void *b = malloc(32);
    void *c = malloc(8);

    uint32_t size_after_bc = h->max - h->s;
    printf("heap size after b,c: %x\n", size_after_bc);

    // b and c should fit in the leftover free block from morebytes'
    // MIN_ALLOC, so no additional growth should occur.
    if(size_after_bc != size_after_a){
        putstr("FAIL: heap grew unexpectedly for small allocations\n");
    }

    printf("a=%x b=%x c=%x\n", (uint32_t)a, (uint32_t)b, (uint32_t)c);

    if(!a || !b || !c){
        putstr("FAIL: null pointer returned\n");
        return;
    }

    // sanity: non-overlapping (each is at least its requested size apart)
    if((uint32_t)b < (uint32_t)a + 16 && (uint32_t)a < (uint32_t)b + 32){
        putstr("FAIL: a and b overlap\n");
    }
    if((uint32_t)c < (uint32_t)b + 32 && (uint32_t)b < (uint32_t)c + 8){
        putstr("FAIL: b and c overlap\n");
    }

    putstr("PASS: test_malloc_basic\n");
}
#pragma endregion misc