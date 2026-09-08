#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../shell/console.h"
#include "../stdlib/stdio.h"
#include "../stdlib/string.h"
#include "../drivers/kb.h"
#include "../drivers/timer.h"
// #include "../mmu/pmm.h"
// #include "../mmu/vmm.h"
#include "../mmu/heap.h"
#include <cpuid.h>
#include "../multitasking/task.h"
#include "../shell/task_shell.h"  //cmd_ps / cmd_kill / cmd_status wrap these
void init_shell(void);
void readline(char *buf, int max);
int  parse_args(char *line, char **argv, int max_args);
void shell_exec(char *line);
void shell_clear(void);

void update_history(char *line);
void get_prev_cmd(void);
void get_next_cmd(void);

void cmd_help  (int argc, char **argv);
void cmd_echo  (int argc, char **argv);
void cmd_darud (int argc, char **argv);
void cmd_lsh   (int argc, char **argv);
void cmd_meminfo (int argc, char **argv);
void cmd_memmap(int argc, char **argv);
void cmd_fetch (int argc, char **argv);
void cmd_exit  (int argc, char **argv);

void cmd_pmmtest(int argc, char **argv);
void cmd_vmmtest(int argc, char **argv);
void cmd_heaptest(int argc, char **argv);

void cmd_ps    (int argc, char **argv);
void cmd_kill  (int argc, char **argv);
void cmd_status(int argc, char **argv);

void print_banner(void);
void print_prompt(void);

typedef void (*cmd_func)(int argc, char **argv);

typedef struct {
    const char *name;
    cmd_func    ptr;
    const char *help_msg;
} command_t;

static const command_t commands[] = {
    { "help",  cmd_help,  "Explains the operations of different commands"              },
    { "echo",  cmd_echo,  "Prints the arguments following the command"                 },
    { "lsh",   cmd_lsh,   "Prints the (at most 8) most recent commands"               },
    {"meminfo", cmd_meminfo, "Prints a cheat sheet of the Os' mmu"}, 
    //{"pmmtest", cmd_pmmtest, "testing the pmm"},
    //{"vmmtest", cmd_vmmtest, "testing the vmm"},
    //{"heaptest", cmd_heaptest, "tests the kernel heap"},
    { "memmap",   cmd_memmap,   "Dumps the Limine physical memory map"             },
    { "fetch", cmd_fetch, "it ain't a real OS without fetch(I love systen diagnostics)"},
    { "ps",     cmd_ps,     "Lists every task's pid, name, state, and CPU time"        },
    { "kill",   cmd_kill,   "Kills a task by pid - usage: kill <pid>"                   },
    { "status", cmd_status, "Shows detailed status for one task - usage: status <pid>"  },
    { "exit",  cmd_exit,  "Gracefully halts the CPU"                                   }
};

const char *memmap_type_str(uint64_t type);