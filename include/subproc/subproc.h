#ifndef HEADER_SUBPROC_H
#define HEADER_SUBPROC_H

#include <sys/types.h>

#define SPIO_PTY -1
#define SPIO_PIPE -2

typedef struct
{
	int fd_in;
	int fd_out;
	int fd_err;
	int returncode;
	bool is_alive;
	pid_t pid;
} subproc;

/**
 * @brief Opens a new subprocess
 * 
 * @param executable the executable file's name
 * @param argv
 * @param envp
 * @param fd_in stdin of the child: either SPIO_PIPE, SPIO_PTY, or a file descriptor
 * @param fd_out stdout of the child
 * @param fd_err stderr of the child
 * @return a pointer to the subproc object containing information about the created child
 */
subproc *sp_open(char *executable, char *argv[], char *envp[], int fd_in, int fd_out, int fd_err);

#endif	// HEADER_SUBPROC_H
