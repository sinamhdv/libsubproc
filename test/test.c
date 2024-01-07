#include <stdio.h>
#include "subproc/subproc.h"
#include "test_utils.h"

char *rev_argv[] = {"/usr/bin/rev", NULL};

void test_signals(void)
{
	puts("\ntest_signals():\n===========================");
	subproc *sp = sp_open(rev_argv[0], rev_argv, NULL, SPIO_PIPE, SPIO_PTY, SPIO_PTY);
	assert(sp->fd_out == sp->fd_err);
	assert(sp->is_alive == true);
	assert(sp->pid > 0);

	log(sp->fd_in);
	log(sp->fd_out);
	log(sp->fd_err);
	log(sp->is_alive);
	log(sp->returncode);
	log(sp->pid);

	pid_t child_pid = sp->pid;
	pid_t my_pid = getpid();
	log(my_pid);

	dump_fds(my_pid);	// should only have 0/1/2 and fd_in/fd_out/fd_err of the child
	dump_fds(sp->pid);	// should only have 0/1/2

	assert(sp_wait(sp, WNOHANG) == 0);

	assert(sp_send_signal(sp, SIGSTOP) == 0);
	assert(sp_wait(sp, WNOHANG) == 0);
	int wstatus;
	assert(waitpid(sp->pid, &wstatus, WUNTRACED) != -1);
	assert(WIFSTOPPED(wstatus));	// test if the child stopped

	assert(sp_send_signal(sp, SIGCONT) == 0);
	assert(sp_wait(sp, WNOHANG) == 0);
	assert(waitpid(sp->pid, &wstatus, WCONTINUED) != -1);
	assert(!WIFSTOPPED(wstatus));	// test if the child continued

	assert(sp_kill(sp) == 0);
	assert(sp_wait(sp, 0) == 1);
	assert(kill(child_pid, 0) == -1);	// test if the child terminated
	assert(sp->is_alive == false);
	sp_free(sp);

	dump_fds(my_pid);	// should be only 0/1/2 now
}

int main(void)
{
	test_signals();
	return 0;
}
