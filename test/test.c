#include "subproc/subproc.h"
#include "test_utils.h"
#include <unistd.h>

void test_signals(void)
{
	puts("\ntest_signals():");
	puts("===========================");
	char *rev_argv[] = {"/usr/bin/rev", NULL};
	subproc *sp = sp_open(rev_argv[0], rev_argv, NULL, SPIO_PIPE, SPIO_PTY, SPIO_PTY);
	assert(sp != NULL);
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

void test_redirection(void)
{
	puts("\ntest_redirection():");
	puts("===========================");
	
	char *cat_argv[] = {"/usr/bin/cat", NULL};
	char *rev_argv[] = {"/usr/bin/rev", NULL};
	subproc *cat_sp = sp_open(cat_argv[0], cat_argv, NULL, SPIO_PIPE, SPIO_PIPE, SPIO_PIPE);
	subproc *rev_sp = sp_open(rev_argv[0], rev_argv, NULL, cat_sp->fd_out, SPIO_PIPE, SPIO_PIPE);
	assert(cat_sp != NULL);
	assert(rev_sp != NULL);

	dump_fds(getpid());
	dump_fds(cat_sp->pid);
	dump_fds(rev_sp->pid);

	assert(rev_sp->fd_in == cat_sp->fd_out);
	
	assert(write(cat_sp->fd_in, "abcd", 4) == 4);
	assert(sp_close(cat_sp) == 0);
	assert(sp_wait(cat_sp, 0) == 1);
	log(cat_sp->returncode);
	sp_free(cat_sp);

	char output_buf[10] = {};
	assert(read(rev_sp->fd_out, output_buf, sizeof(output_buf)) == 4);
	assert(strcmp(output_buf, "dcba") == 0);
	sp_free(rev_sp);

	dump_fds(getpid());
}

int main(void)
{
	test_signals();
	test_redirection();
	return 0;
}
