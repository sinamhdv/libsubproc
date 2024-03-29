#include "subproc/errors.h"
#include "subproc/subproc.h"
#include "test_utils.h"

void test_signals(void)
{
	puts("\ntest_signals():");
	puts("===========================");
	char *rev_argv[] = {"/usr/bin/rev", NULL};
	subproc *sp = sp_open(rev_argv[0], rev_argv, NULL, SPIO_PIPE, SPIO_PTY, SPIO_PTY);
	assert(sp != NULL);
	assert(sp->fd_out == sp->fd_err);
	assert(sp->_waited == false);
	assert(sp->returncode == 0);
	assert(sp->pid > 0);

	log(sp->fd_in);
	log(sp->fd_out);
	log(sp->fd_err);
	log(sp->pid);

	pid_t my_pid = getpid();
	log(my_pid);

	dump_fds(my_pid);	// EXPECTED: 0/1/2, one pipe, and one pty
	dump_fds(sp->pid);	// EXPECTED: 0/1/2 only: 0 to pipe, 1 & 2 to pty

	assert(sp_wait(sp, WNOHANG) == 0);
	assert(sp->_waited == false);

	assert(sp_send_signal(sp, SIGSTOP) == 0);
	assert(sp_wait(sp, WNOHANG) == 0);
	assert(sp->_waited == false);
	int wstatus;
	assert(waitpid(sp->pid, &wstatus, WUNTRACED) != -1);
	assert(WIFSTOPPED(wstatus));	// test if the child stopped

	assert(sp_send_signal(sp, SIGCONT) == 0);
	assert(sp_wait(sp, WNOHANG) == 0);
	assert(sp->_waited == false);
	assert(waitpid(sp->pid, &wstatus, WCONTINUED) != -1);
	assert(!WIFSTOPPED(wstatus));	// test if the child continued

	assert(sp_kill(sp) == 0);
	assert(sp->_waited == false);
	assert(sp_wait(sp, 0) == 1);
	assert(sp->_waited == true);
	sp_free(sp);

	dump_fds(my_pid);	// EXPECTED: 0/1/2 only
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

	dump_fds(getpid());	// EXPECTED: 0/1/2 and 5 pipes
	dump_fds(cat_sp->pid);	// EXPECTED: 0/1/2 only
	dump_fds(rev_sp->pid);	// EXPECTED: 0/1/2 only

	assert(rev_sp->fd_in == cat_sp->fd_out);
	
	assert(write(cat_sp->fd_in, "abcd", 4) == 4);
	assert(sp_close(cat_sp) == 0);
	assert(sp_wait(cat_sp, 0) == 1);
	assert(sp_kill(cat_sp) == -1);
	assert(errno == ESRCH);
	assert(cat_sp->returncode == 0);
	sp_free(cat_sp);

	char output_buf[10] = {};
	assert(read(rev_sp->fd_out, output_buf, sizeof(output_buf)) == 4);
	assert(strcmp(output_buf, "dcba") == 0);
	sp_free(rev_sp);

	dump_fds(getpid());	// EXPECTED: 0/1/2 only
}

void test_spio_options(void)
{
	puts("\ntest_spio_options():");
	puts("===========================");

	// SPIO_DEV_NULL and SPIO_PARENT	
	char *argv1[] = {"/usr/bin/cat", "/nonexistent", "-", NULL};
	subproc *sp = sp_open(argv1[0], argv1, NULL, SPIO_PIPE, SPIO_PARENT, SPIO_DEVNULL);
	dump_fds(sp->pid);	// EXPECTED: 0/1/2 only: 0 to pipe, 1 to output terminal, 2 to /dev/null
	sp_free(sp);

	// SPIO_STDOUT
	char *argv2[] = {"/usr/bin/cat", "/nonexistent", NULL};
	sp = sp_open(argv2[0], argv2, NULL, SPIO_DEVNULL, SPIO_PIPE, SPIO_STDOUT);
	assert(sp->fd_err == sp->fd_out);
	char buf[128] = {};
	size_t read_cnt = 0;
	while (read_cnt < sizeof(buf) - 1 && strchr(buf, '\n') == NULL)
	{
		size_t cur_read_cnt = read(sp->fd_out, buf + read_cnt, sizeof(buf) - 1 - read_cnt);
		assert(cur_read_cnt > 0);
		read_cnt += cur_read_cnt;
	}
	buf[read_cnt] = 0;
	assert(strstr(buf, "No such file or directory") != NULL);
	sp_free(sp);
}

void test_errors(void)
{
	puts("\ntest_errors():");
	puts("===========================");

	char *argv[] = {"/nonexistent", NULL};
	subproc *sp = sp_open(argv[0], argv, NULL, SPIO_DEVNULL, SPIO_PIPE, SPIO_STDOUT);
	assert(sp_wait(sp, 0) > 0);
	
	assert(sp_wait(sp, 0) == -1);
	assert(sp_errno == ECHILD);
	assert(strcmp(sp_errfunc, "waitpid") == 0);
	sp_perror("msg1");

	assert(sp_kill(sp) == -1);
	assert(sp_errno == ESRCH);
	assert(strcmp(sp_errfunc, "sp_send_signal") == 0);
	sp_perror("msg2");

	assert(sp_close(sp) == -1);
	assert(sp_errno == EBADF);
	assert(strcmp(sp_errfunc, "sp_close") == 0);
	sp_perror("msg3");

	sp_free(sp);

	assert(sp_open(argv[0], argv, NULL, SPIO_STDOUT, SPIO_PIPE, SPIO_PIPE) == NULL);
	assert(sp_errno == EINVAL);
	assert(strcmp(sp_errfunc, "sp_open") == 0);
	sp_perror("msg4");
}

int main(void)
{
	test_signals();
	test_redirection();
	test_spio_options();
	test_errors();
	return 0;
}
