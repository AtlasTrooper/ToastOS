#pragma once

#include "../vga.h"
#include "../serial.h"
#include "../io.h"
#include "../kb.h"
#include "../timer.h"
#include "../memoryMan/memory.h"
#include "../stdlib/malloc.h"

//Base shell functions

void init_shell();
void readline(char *buf, int max);
int parse_args(char *line, char **argv, int max_args);
void shell_exec(char *line);
void shell_clear();
//History functions

void update_history(char *line);
void get_prev_cmd();
void get_next_cmd();

//Commands
void cmd_help(int argc, char **argv);
void cmd_echo(int argc, char **argv);
void cmd_darud(int argc, char **argv);
void cmd_lsh(int argc, char **argv);
void cmd_exit(int argc, char **argv);
void cmd_memstat(int argc, char **argv);
void test_malloc_basic(int argc, char **argv);

//decor(totally necessary)
void print_banner();
void print_prompt();

typedef void (*cmd_func) (int argc, char **argv);


typedef struct command_t {
    const char* name;
    cmd_func ptr;
    const char* help_msg;

} command_t;

static const command_t commands[] = {
    {"help", cmd_help, "Explains the operations of different commands"},
    {"echo", cmd_echo, "Prints the line following the command word" },
    {"darud", cmd_darud, ""},
    {"lsh", cmd_lsh, "Prints the the (at most 8) most recent commands the user has used"},
    {"memst", cmd_memstat, "Prints information on the kernel_heap"},
    {"tst", test_malloc_basic, "test"},
    {"exit", cmd_exit, "Gracefully shuts down and exits the os"}

};