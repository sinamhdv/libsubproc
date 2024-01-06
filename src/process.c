#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include "subproc/subproc.h"

/**
 * @brief Creates the pipes and ptys to be used by the child process
 * 
 * @param fd_status an array showing what we should do for each standard fd
 * @param used_fds the fd array to fill for stdin/stdout/stderr and read/write end
 * @param pty_master the master side of the created pty, if used
 * @return true on success, false on error
 */
static bool create_used_fds(int fd_status[3], int used_fds[3][2], int pty_master)
{
	for (int i = 0; i < 3; i++)
	{
		if (fd_status[i] == SPIO_PTY)
			used_fds[i][0] = used_fds[i][1] = pty_master;
		else if (fd_status[i] == SPIO_PIPE)
		{
			if (pipe2(used_fds[i], O_CLOEXEC) == -1)
				return false;
		}
		else
			used_fds[i][0] = used_fds[i][1] = fd_status[i];
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
			if (dup2(used_fds[i][(i == 0 ? 0 : 1)], i) == -1)
				return false;
		}
		else if (fd_status[i] == SPIO_PTY)
		{
			
		}
		else
		{
			
		}
	}
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
	int used_fds[3][2];
	bool use_pty = (fd_status[0] == SPIO_PTY ||
					fd_status[1] == SPIO_PTY ||
					fd_status[2] == SPIO_PTY);
	int pty[2] = {-1, -1};	// master/slave

	if (use_pty && !create_pty(pty))
		goto fail;
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
