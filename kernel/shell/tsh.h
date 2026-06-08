#pragma once

#include "../vga.h"
#include "../serial.h"
#include "../io.h"
#include "../kb.h"
#include "../memoryMan/memory.h"

typedef void (*cmd_func) (int argc, char **argv);


typedef struct command_t {
    const char* name;
    cmd_func ptr;
    const char* help_msg;

} command_t;

static const command_t commands[] = {
    {}
};

//Base shell functions

void init_shell();
void readline(char *buf, int max);
int parse_args(char *line, char **argv, int max_args);
void shell_exec(char *line);

//History functions

void update_history(char * arg);
char * get_prev_cmd();

//Commands


//decor(totally necessary)
void print_banner();
void print_prompt();