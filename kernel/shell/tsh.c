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
        putstr("No command given — printing all commands\n\n");
        putstr("            CMD LIST            \n");
        putstr("--------------------------------\n");
        for (int i = 0; i < cmd_count; i++) {
            if (strlen(commands[i].help_msg) > 0)
                printf("  %-8s : %s\n", commands[i].name, commands[i].help_msg);
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
    /* play_sandstorm() — wire up once audio driver is ported */
    putstr("darud: sandstorm not yet ported to 64-bit\n");
}

void cmd_lsh(int argc, char **argv) {
    (void)argc; (void)argv;
    int count = history_count % HISTORY_MAX;
    if (count == 0 && history_count > 0) count = HISTORY_MAX;
    for (int i = 0; i < count; i++)
        printf("  %d : %s\n", i, cmd_history[i]);
}

void cmd_exit(int argc, char **argv) {
    (void)argc; (void)argv;
    putstr("Halting CPU — adios amigo!\n");
    asm volatile("cli; hlt");
}

void print_banner(void) {
    console_set_color(CON_LIGHT_BROWN, CON_BLACK);
    putstr("=================================================================\n");
    putstr(" ToastOS x86_64  |  tsh  |  est. 2026  Tomer Wiesel\n");
    putstr(" (eden was here :)\n");
    putstr("=================================================================\n");
    console_set_color(CON_LIGHT_GREY, CON_BLACK);
}

void print_prompt(void) {
    console_set_color(CON_LIGHT_GREEN, CON_BLACK);
    putstr("tsh");
    console_set_color(CON_WHITE, CON_BLACK);
    putstr("> ");
    console_set_color(CON_LIGHT_GREY, CON_BLACK);
}