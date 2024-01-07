#ifndef HEADER_SUBPROC_H
#define HEADER_SUBPROC_H

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
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
 * @return a pointer to the subproc object for the created child or NULL on error
 */
subproc *sp_open(char *executable, char *argv[], char *envp[], int fd_in, int fd_out, int fd_err);

int sp_send_signal(subproc *sp, int sig);

int sp_kill(subproc *sp);

// TEMP DOC: returns 1 if terminated, -1 on error, 0 if child not terminated in non-blocking calls
int sp_wait(subproc *sp, int options);

int sp_close(subproc *sp);

void sp_free(subproc *sp);

#endif	// HEADER_SUBPROC_H
