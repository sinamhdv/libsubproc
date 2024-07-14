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
	bool _waited;
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

/**
 * @brief send a signal to the subprocess
 * 
 * @param sp subprocess to send the signal to
 * @param sig signal number
 * @return 0 on success, -1 on error
 */
int sp_send_signal(subproc *sp, int sig);

/**
 * @brief send SIGKILL to subprocess sp
 * 
 * @param sp 
 * @return 0 on success, -1 on error
 */
int sp_kill(subproc *sp);

// TEMP DOC: returns 1 if terminated, -1 on error, 0 if child not terminated in non-blocking calls
/**
 * @brief wait on the subprocess child
 * 
 * @param sp 
 * @param options flags for the waitpid syscall (e.g. WNOHANG for non-blocking wait)
 * @return returns 1 and sets sp->return_code if terminated. Returns -1 on error and 0 for no status change in WNOHANG option
 */
int sp_wait(subproc *sp, int options);

/**
 * @brief close the stdin of the subprocess and free its buffer
 * 
 * @param sp 
 * @return 0 on success, -1 on error
 */
int sp_close(subproc *sp);

/**
 * @brief free and clean up internal structures in a subproc struct
 * 
 * @param sp
 */
void sp_free(subproc *sp);

#endif	// SP_HEADER_SUBPROC_H
