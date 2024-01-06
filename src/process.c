#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include "subproc/subproc.h"

/**
 * @brief Creates the pipes and ptys to be used by the child process
 * 
 * @param fd_status an array showing what we should do for each standard fd
 * @param used_fds the fd array to fill for stdin/stdout/stderr and read/write end
 * @return true on success, false on error
 */
static bool create_used_fds(int fd_status[3], int used_fds[3][2])
{
	int pty_master = -1;

	for (int i = 0; i < 3; i++)
	{
		if (fd_status[i] == SPIO_PTY)
		{
			if (pty_master == -1)
			{
				if ((pty_master = posix_openpt(O_RDWR | O_NOCTTY)) == -1)
					return false;
				if (grantpt(pty_master) == -1)
				{
					close(pty_master);
					return false;
				}
				if (unlockpt(pty_master) == -1)
				{
					close(pty_master);
					return false;
				}
				if (fcntl(pty_master, F_SETFD, FD_CLOEXEC) == -1)
					return false;
				used_fds[i][0] = used_fds[i][1] = pty_master;
			}
		}
		else if (fd_status[i] == SPIO_PIPE)
		{
			if (pipe2(used_fds[i], O_CLOEXEC) == -1)
				return false;
		}
		else
		{
			used_fds[i][0] = used_fds[i][1] = fd_status[i];
		}
	}

	return true;
}

/**
 * @brief Duplicate used fds as child standard io fds
 * 
 * @param fd_status what is the type of each fd (SPIO_PIPE, SPIO_PTY, or an fd)
 * @param used_fds the fds we should duplicate into standard io fds
 * @return true on success, false on failure
 */
static bool duplicate_used_fds(int fd_status[3], int used_fds[3][2])
{
	for (int i = 0; i < 3; i++)
	{
		if (fd_status[i] == SPIO_PIPE)
		{

		}
		else if (fd_status[i] == SPIO_PTY)
		{

		}
		else
		{
			
		}
	}
}

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
subproc *sp_open(char *executable, char *argv[], char *envp[], int fd_in, int fd_out, int fd_err)
{
	// allocate the struct
	subproc *sp = malloc(sizeof(subproc));
	if (sp == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	memset(sp, 0, sizeof(subproc));

	int fd_status[3] = {fd_in, fd_out, fd_err};
	int used_fds[3][2];
	if (!create_used_fds(fd_status, used_fds))
		goto fail;

	pid_t pid = fork();
	if (pid == -1)
		goto fail;
	if (pid == 0)	// child
	{
		if (!duplicate_used_fds(fd_status, used_fds))
			exit(1);
		if (execve(executable, argv, envp) == -1)
			exit(1);
	}
	else	// parent
	{

	}

	return sp;
fail:
	free(sp);
	return NULL;
}
