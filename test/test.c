#include <stdio.h>
#include <unistd.h>
#include "subproc/subproc.h"
#include "test_utils.h"

void test_pipe_pty(void)
{
	char *argv[] = {"/usr/bin/rev", NULL};
	subproc *sp = sp_open(argv[0], argv, NULL, SPIO_PIPE, SPIO_PTY, SPIO_PTY);
	assert(sp->fd_out == sp->fd_err);
	assert(sp->is_alive == true);
	assert(sp->pid > 0);

	log(sp->fd_in);
	log(sp->fd_out);
	log(sp->fd_err);
	log(sp->is_alive);
	log(sp->returncode);
	log(sp->pid);

	pid_t my_pid = getpid();
	log(my_pid);

	dump_fds(my_pid);

	assert(sp_kill(sp) == 0);
	sp_free(sp);
}

int main(void)
{
	test_pipe_pty();
	return 0;
}
