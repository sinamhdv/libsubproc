#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "subproc/subproc.h"

subproc *sp_open(char *executable, char *argv[], char *envp[], int fd_in, int fd_out, int fd_err)
{
	// allocate the struct
	subproc *sp = malloc(sizeof(subproc));
	if (sp == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	memset(sp, 0, sizeof(subproc));
	
	// create the needed pipes/ptys
	int fd_status[3] = {fd_in, fd_out, fd_err};
	int used_fds[3][2];	// [0] => parent, [1] => child
	int pty_master = -1;

	for (int i = 0; i < 3; i++) {
		if (fd_status[i] == SPIO_PTY) {
			if (pty_master == -1) {
				if ((pty_master = posix_openpt(O_RDWR | O_NOCTTY)) == -1)
					goto fail;
				if (grantpt(pty_master) == -1) {
					close(pty_master);
					goto fail;
				}
				if (unlockpt(pty_master) == -1) {
					close(pty_master);
					goto fail;
				}
				if (fcntl(pty_master, F_SETFD, FD_CLOEXEC) == -1)
					goto fail;
				used_fds[i][0] = used_fds[i][1] = pty_master;
			}
		} else if (fd_status[i] == SPIO_PIPE) {
			if (pipe2(used_fds[i], O_CLOEXEC) == -1) goto fail;
		} else {
			used_fds[i][0] = used_fds[i][1] = fd_status[i];
		}
	}

	pid_t pid = fork();
	if (pid == -1) goto fail;
	if (pid == 0) {	// child
		
	} else {	// parent

	}

	return sp;
fail:
	free(sp);
	return NULL;
}
