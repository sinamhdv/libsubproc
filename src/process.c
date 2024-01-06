#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include "subproc/subproc.h"

static bool create_pipes(int fd_status[3], int pipes[3][2])
{
	for (int i = 0; i < 3; i++)
	{
		if (fd_status[i] == SPIO_PIPE)
		{
			if (pipe2(pipes[i], O_CLOEXEC) == -1)
				return false;
		}
	}
	return true;
}

static bool duplicate_fds(int fd_status[3], int pipes[3][2], int pty_slave)
{
	for (int i = 0; i < 3; i++)
	{
		if (fd_status[i] == SPIO_PIPE)
		{
			if (dup2(pipes[i][(i == 0 ? 0 : 1)], i) == -1)
				return false;
		}
		else if (fd_status[i] == SPIO_PTY)
		{
			if (dup2(pty_slave, i) == -1)
				return false;
		}
		else
		{
			if (dup2(fd_status[i], i) == -1)
				return false;
		}
	}
	return true;
}

static int create_pty_master(void)
{
	int pty_master;
	if ((pty_master = posix_openpt(O_RDWR | O_NOCTTY)) == -1)
		return -1;
	if (grantpt(pty_master) == -1)
		goto fail;
	if (unlockpt(pty_master) == -1)
		goto fail;
	if (fcntl(pty_master, F_SETFD, FD_CLOEXEC) == -1)
		goto fail;
	
	return pty_master;
fail:
	close(pty_master);
	return -1;
}

static int create_pty_slave(int pty_master)
{
	int pty_slave;
	char slave_name[256];
	if (ptsname_r(pty_master, slave_name, sizeof(slave_name)))
		return -1;
	int pty_slave = open(slave_name, O_RDWR | O_CLOEXEC);
	if (pty_slave == -1) return -1;
	if (!set_tty_options(pty_slave))
		goto fail;
	if (setsid() == -1)
		goto fail;
	if (ioctl(pty_slave, TIOCSCTTY, 0) == -1)
		goto fail;

	return pty_slave;
fail:
	close(pty_slave);
	return -1;
}

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
	int pipes[3][2];
	bool use_pty = (fd_status[0] == SPIO_PTY ||
					fd_status[1] == SPIO_PTY ||
					fd_status[2] == SPIO_PTY);
	int pty_master = -1;

	if (use_pty)
	{
		pty_master = create_pty_master();
		if (pty_master == -1)
			goto fail;
	}
	if (!create_pipes(fd_status, pipes))
		goto fail;

	pid_t pid = fork();
	if (pid == -1)
		goto fail;
	if (pid == 0)	// child
	{
		int pty_slave = -1;
		if (use_pty)
		{
			pty_slave = create_pty_slave(pty_master);
			if (pty_slave == -1)
				exit(1);
		}
		if (!duplicate_fds(fd_status, pipes, pty_slave))
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
