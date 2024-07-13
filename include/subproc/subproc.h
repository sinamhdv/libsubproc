#ifndef SP_HEADER_SUBPROC_H
#define SP_HEADER_SUBPROC_H

#define _GNU_SOURCE
#include <stdio.h>
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
#include <poll.h>

#include "subproc/errors.h"
#include "subproc/io.h"

#define SPIO_PTY -1		// open a pty
#define SPIO_PIPE -2	// open a pipe
#define SPIO_DEVNULL -3	// redirect to /dev/null
#define SPIO_STDOUT -4	// redirecting stderr to stdout
#define SPIO_PARENT	-5	// leave this fd the same as parent

typedef struct subproc
{
	int fds[3];
	struct sp_io_buffer buf[3];
	int returncode;
	pid_t pid;
	bool _waited;	// XXX: this should not be changed by users
} subproc;

/**
 * @brief Opens a new subprocess
 * 
 * @param sp pointer to the subproc struct to use
 * @param executable the executable file's name
 * @param argv
 * @param envp
 * @param fd_values values (or special SPIO_... flags) for fds of the subprocess
 * @param bufsize buffer size for each fd (or 0 for unbuffered)
 * @return 0 on success, -1 on error (and sets sp_errno).
 */
int sp_open(subproc *sp, char *executable, char *argv[], char *envp[], int fd_values[3], size_t bufsize[3]);

int sp_send_signal(subproc *sp, int sig);

int sp_kill(subproc *sp);

// TEMP DOC: returns 1 if terminated, -1 on error, 0 if child not terminated in non-blocking calls
int sp_wait(subproc *sp, int options);

int sp_close(subproc *sp);

void sp_free(subproc *sp);

#endif	// SP_HEADER_SUBPROC_H
