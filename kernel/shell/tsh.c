#include "tsh.h"

/*
max size of a single command, currently hardcoded 
and stack allocated soon to be malloced and heap allocated
*/
#define KBUF_SIZE 256
#define ARGC_MAX 16
#define HISTORY_MAX 8

static int history_count = 0; 
static char cmd_history[HISTORY_MAX][KBUF_SIZE];
static char buf[KBUF_SIZE];
static int prev_command_index = 0;
void init_shell() {
    // char buf[KBUF_SIZE];
    print_banner();

    while(1) {
        print_prompt();
        readline(buf, KBUF_SIZE);
        if(!buf[0]) {continue;}

        update_history(buf);
        shell_exec(buf);

    }
}

void readline(char *buf, int max){
    int i = 0;
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
                buf[i] == '\0';
                putchar('\n');
                return;
            case '\b':
                if (i > 0) {
                    i--;
                    buf[i] = '\0';
                    putchar('\b');
                    //putchar(' ');
                    //putchar('\b');
                }
                continue;

            default:
                break;
        }
        if (i < max -1) {
            buf[i++] = c;
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
    memset(buf, 0, KBUF_SIZE);
    terminal_clear();
    print_prompt();
}

void update_history(char *line) {
    if(!line[0]) return;
    prev_command_index = (history_count%HISTORY_MAX);
    strcpy(cmd_history[(history_count++)%HISTORY_MAX], line);
}

void get_prev_cmd() {
    //debug_print("UP\n");
    memset(buf, 0, KBUF_SIZE);
    clearCurrentLine();
    print_prompt();
    strcpy(buf, cmd_history[prev_command_index]);
    putstr(buf);
    if (strlen(cmd_history[(prev_command_index-1)%HISTORY_MAX]) != 0) {
        prev_command_index = (prev_command_index-1)%HISTORY_MAX;
    }
}


void cmd_help(int argc, char **argv) {
    if (argc < 2) {
        putstr("No command given, printing all commands\n\n");
        putstr("                 CMD LIST                   ");
        putstr("\n---------------------------------------\n");
        for (int i = 0; i < sizeof(commands)/sizeof(command_t); i++) {
            printf("%s : %s \n", commands[i].name, commands[i].help_msg);
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
