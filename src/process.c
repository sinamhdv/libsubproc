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
		else if (fd_status[i] == SPIO_STDOUT)
		{
			if (dup2(1, 2) == -1)
				return false;
		}
		else if (fd_status[i] == SPIO_DEVNULL)
		{
			int dev_null_fd = open("/dev/null", O_CLOEXEC | (i == 0 ? O_RDONLY : O_WRONLY));
			if (dev_null_fd == -1) return false;
			if (dup2(dev_null_fd, i) == -1)
				return false;
		}
		else if (fd_status[i] != SPIO_PARENT)	// custom fd
		{
			if (dup2(fd_status[i], i) == -1 ||
				close(fd_status[i]) == -1)
				return false;
		}
	}
	return true;
}

static bool assign_fds(int fd_status[3], int pipes[3][2], int pty_master, int fd_assignments[3])
{
	for (int i = 0; i < 3; i++)
	{
		if (fd_status[i] == SPIO_PIPE)
		{
			int idx = (i == 0 ? 1 : 0);
			fd_assignments[i] = pipes[i][idx];
			if (close(pipes[i][1 - idx]) == -1)
				return false;
		}
		else if (fd_status[i] == SPIO_PTY)
			fd_assignments[i] = pty_master;
		else if (fd_status[i] == SPIO_STDOUT)
			fd_assignments[i] = fd_assignments[1];
		else if (fd_status[i] == SPIO_DEVNULL)
			fd_assignments[i] = -1;
		else if (fd_status[i] == SPIO_PARENT)
			fd_assignments[i] = -1;
		else	// custom fd
			fd_assignments[i] = fd_status[i];
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

static bool set_tty_options(int pty_slave)
{
	struct termios tty_opts;
	if (tcgetattr(pty_slave, &tty_opts) == -1) return false;
	cfmakeraw(&tty_opts);
	if (tcsetattr(pty_slave, TCSANOW, &tty_opts) == -1) return false;
	return true;
}

static int create_pty_slave(int pty_master)
{
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
	if (fd_in == SPIO_STDOUT || fd_out == SPIO_STDOUT)
	{
		errno = EINVAL;
		return NULL;
	}

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

	pid_t child_pid = fork();
	if (child_pid == -1)
		goto fail;
	if (child_pid == 0)	// child
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
		int fd_assignments[3];
		if (!assign_fds(fd_status, pipes, pty_master, fd_assignments))
		{
			close(pty_master);
			for (int i = 0; i < 3; i++)
			{
				if (fd_status[i] == SPIO_PIPE)
				{
					close(pipes[i][0]);
					close(pipes[i][1]);
				}
			}
			kill(child_pid, SIGKILL);
			waitpid(child_pid, NULL, 0);
			goto fail;
		}
		sp->fd_in = fd_assignments[0];
		sp->fd_out = fd_assignments[1];
		sp->fd_err = fd_assignments[2];
		sp->pid = child_pid;
		sp->_waited = false;
	}

	return sp;
fail:
	free(sp);
	return NULL;
}

int sp_send_signal(subproc *sp, int sig)
{
	if (sp->_waited)
	{
		errno = ESRCH;
		return -1;
	}
	return kill(sp->pid, sig);
}

int sp_kill(subproc *sp)
{
	int ret_val = sp_send_signal(sp, SIGKILL);
	return ret_val;
}

int sp_wait(subproc *sp, int options)
{
	int status;
	int ret_val = waitpid(sp->pid, &status, options);
	if (ret_val == -1 || ret_val == 0)
		return ret_val;
	if (WIFEXITED(status))
		sp->returncode = WEXITSTATUS(status);
	sp->_waited = true;
	return 1;
}

int sp_close(subproc *sp)
{
	if (sp->fd_in != -1)
	{
		int ret_val = close(sp->fd_in);
		if (ret_val == -1) return -1;
		sp->fd_in = -1;
		return ret_val;
	}
	errno = EBADF;
	return -1;
}

void sp_free(subproc *sp)
{
	if (!sp->_waited)
	{
		sp_kill(sp);
		sp_wait(sp, 0);
	}
	if (sp->fd_in != -1) close(sp->fd_in);
	if (sp->fd_out != -1) close(sp->fd_out);
	if (sp->fd_err != -1) close(sp->fd_err);
	free(sp);
}
