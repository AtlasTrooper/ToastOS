#pragma once
#include "../multitasking/task.h"

void shell_cmd_ps(void);
int  shell_cmd_kill(uint64_t pid);
void shell_cmd_status(uint64_t pid);