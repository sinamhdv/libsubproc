#ifndef HEADER_SUBPROC_H
#define HEADER_SUBPROC_H

#include <sys/types.h>

#define SPIO_PTY -1
#define SPIO_PIPE -2

typedef struct {
	int fd_in;
	int fd_out;
	int fd_err;
	int returncode;
	pid_t pid;
} subproc;

#endif	// HEADER_SUBPROC_H
