#ifndef HEADER_TEST_UTILS
#define HEADER_TEST_UTILS

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "subproc/subproc.h"

#define log(x) (printf(#x " = %d\n", (x)))

// dump all file descriptors of a process
void dump_fds(pid_t pid)
{
	char cmd[64];
	sprintf(cmd, "/usr/bin/ls -l /proc/%d/fd", pid);
	system(cmd);
}

// show a warning message with colors
void show_warning(char *msg)
{
	printf("\033[33m%s\033[0m\n", msg);
}

#endif	// HEADER_TEST_UTILS
