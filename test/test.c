#include "subproc/subproc.h"
#include "test_utils.h"

void test_signals(void)
{
	puts("\ntest_signals():");
	puts("===========================");
	char *rev_argv[] = {"/usr/bin/rev", NULL};
	subproc *sp = malloc(sizeof(subproc));
	assert(sp_open(sp, rev_argv[0], rev_argv, NULL,
		(int[3]){SPIO_PIPE, SPIO_PTY, SPIO_PTY},
		(size_t[3]){0, 0, 0}) == 0);
	assert(sp != NULL);
	assert(sp->fds[1] == sp->fds[2]);
	assert(sp->_waited == false);
	assert(sp->returncode == 0);
	assert(sp->pid > 0);

	log(sp->fds[0]);
	log(sp->fds[1]);
	log(sp->fds[2]);
	log(sp->pid);

	pid_t my_pid = getpid();
	log(my_pid);

	dump_fds(my_pid);
	show_warning("EXPECTED: 0/1/2, one pipe, and one pty");
	dump_fds(sp->pid);
	show_warning("EXPECTED: 0/1/2 only: 0 to pipe, 1 & 2 to pty");

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
	free(sp);

	dump_fds(my_pid);
	show_warning("EXPECTED: 0/1/2 only (all to output terminal)");
}

void test_redirection(void)
{
	puts("\ntest_redirection():");
	puts("===========================");
	
	char *cat_argv[] = {"/usr/bin/cat", NULL};
	char *rev_argv[] = {"/usr/bin/rev", NULL};
	subproc cat_sp, rev_sp;
	assert(sp_open(&cat_sp, cat_argv[0], cat_argv, NULL,
		(int[3]){SPIO_PIPE, SPIO_PIPE, SPIO_PIPE},
		(size_t[3]){0, 0, 0}) == 0);
	assert(sp_open(&rev_sp, rev_argv[0], rev_argv, NULL,
		(int[3]){cat_sp.fds[1], SPIO_PIPE, SPIO_PIPE},
		(size_t[3]){0, 0, 0}) == 0);

	dump_fds(getpid());
	show_warning("EXPECTED: 0/1/2 and 5 pipes");
	dump_fds(cat_sp.pid);
	show_warning("EXPECTED: 0/1/2 only (all to pipes)");
	dump_fds(rev_sp.pid);
	show_warning("EXPECTED: 0/1/2 only (all to pipes)");

	assert(rev_sp.fds[0] == cat_sp.fds[1]);
	
	assert(write(cat_sp.fds[0], "abcd", 4) == 4);
	assert(sp_close(&cat_sp) == 0);
	assert(sp_wait(&cat_sp, 0) == 1);
	assert(sp_kill(&cat_sp) == -1);
	assert(errno == ESRCH);
	assert(cat_sp.returncode == 0);
	sp_free(&cat_sp);

	char output_buf[10] = {};
	assert(read(rev_sp.fds[1], output_buf, sizeof(output_buf)) == 4);
	assert(strcmp(output_buf, "dcba") == 0);
	sp_free(&rev_sp);

	dump_fds(getpid());
	show_warning("EXPECTED: 0/1/2 only (all to output terminal)");
}

void test_spio_options(void)
{
	puts("\ntest_spio_options():");
	puts("===========================");

	// SPIO_DEV_NULL and SPIO_PARENT	
	char *argv1[] = {"/usr/bin/cat", "/nonexistent", "-", NULL};
	subproc sp;
	assert(sp_open(&sp, argv1[0], argv1, NULL,
		(int[3]){SPIO_PIPE, SPIO_PARENT, SPIO_DEVNULL},
		(size_t[3]){0, 0, 0}) == 0);
	dump_fds(sp.pid);
	show_warning("EXPECTED: 0/1/2 only: 0 to pipe, 1 to output terminal, 2 to /dev/null");
	sp_free(&sp);

	// SPIO_STDOUT
	char *argv2[] = {"/usr/bin/cat", "/nonexistent", NULL};
	assert(sp_open(&sp, argv2[0], argv2, NULL,
		(int[3]){SPIO_DEVNULL, SPIO_PIPE, SPIO_STDOUT},
		(size_t[3]){0, 0, 0}) == 0);
	assert(sp.fds[2] == sp.fds[1]);
	char buf[128] = {};
	size_t read_cnt = 0;
	while (read_cnt < sizeof(buf) - 1 && strchr(buf, '\n') == NULL)
	{
		size_t cur_read_cnt = read(sp.fds[1], buf + read_cnt, sizeof(buf) - 1 - read_cnt);
		assert(cur_read_cnt > 0);
		read_cnt += cur_read_cnt;
	}
	buf[read_cnt] = 0;
	assert(strstr(buf, "No such file or directory") != NULL);
	sp_free(&sp);
}

void test_errors(void)
{
	puts("\ntest_errors():");
	puts("===========================");

	char *argv[] = {"/nonexistent", NULL};
	subproc sp;
	assert(sp_open(&sp, argv[0], argv, NULL,
		(int[3]){SPIO_DEVNULL, SPIO_PIPE, SPIO_STDOUT},
		(size_t[3]){0, 0, 0}) == 0);
	assert(sp_wait(&sp, 0) > 0);
	
	assert(sp_wait(&sp, 0) == -1);
	assert(sp_errno == ECHILD);
	assert(strcmp(sp_errfunc, "waitpid") == 0);
	sp_perror("msg1");

	assert(sp_kill(&sp) == -1);
	assert(sp_errno == ESRCH);
	assert(strcmp(sp_errfunc, "sp_send_signal") == 0);
	sp_perror("msg2");

	assert(sp_close(&sp) == -1);
	assert(sp_errno == EBADF);
	assert(strcmp(sp_errfunc, "sp_close") == 0);
	sp_perror("msg3");

	sp_free(&sp);

	assert(sp_open(&sp, argv[0], argv, NULL,
		(int[3]){SPIO_STDOUT, SPIO_PIPE, SPIO_PIPE},
		(size_t[3]){0, 0, 0}) == -1);
	assert(sp_errno == EINVAL);
	assert(strcmp(sp_errfunc, "sp_open") == 0);
	sp_perror("msg4");
}

void test_buffered_io(size_t bufsize)
{
	printf("\ntest_buffered_io(bufsize=%lu):\n", bufsize);
	puts("===========================");

	subproc sp;
	char *argv[] = {"/usr/bin/env", "python3", NULL};
	assert(sp_open(&sp, argv[0], argv, NULL,
		(int[3]){SPIO_PTY, SPIO_PTY, SPIO_STDOUT},
		(size_t[3]){bufsize, bufsize, bufsize}) == 0);
	char buf[1024] = {};
	ssize_t ret;

	ret = sp_recvuntil(&sp, buf, sizeof(buf) - 1, "th", false);
	assert(ret == 4);
	buf[ret] = 0;
	assert(strcmp(buf, "Pyth") == 0);
	if (bufsize >= 10)
	{
		assert(strncmp(sp.buf[1].ptr, "on ", 3) == 0);
		assert(strncmp(sp.buf[1].start, "Python ", 7) == 0);
	}
	ret = sp_recvuntil(&sp, buf, sizeof(buf) - 1, ">>> ", false);
	assert(ret == strlen(buf));
	assert(strcmp(buf + strlen(buf) - 4, ">>> ") == 0);
	
	assert(sp_sends(&sp, "print('hello world!!!')\n") == 0);
	ret = sp_recvline(&sp, buf, sizeof(buf) - 1, false);
	assert(ret > 0);
	buf[ret] = 0;
	assert(ret == strlen(buf));
	assert(strcmp(buf, "hello world!!!\n") == 0);

	assert(sp_sends(&sp, "__import__('sys').stdout.buffer.write(b'string\\0with\\0nulls\\n')\n") == 0);
	ret = sp_recvuntil(&sp, buf, 5, "nulls", false);
	assert(ret == 5);
	buf[ret] = 0;
	assert(strcmp(buf, ">>> s") == 0);
	ret = sp_recvuntil(&sp, buf, 100, "nulls", false);
	assert(ret > 0);
	assert(memcmp(buf, "tring\0with\0nulls", ret) == 0);

	assert(sp_recvuntil(&sp, buf, 100, ">>", false) > 0);
	assert(sp_recvc(&sp, false) == '>');
	assert(sp_recvc(&sp, false) == ' ');
	assert(sp_sends(&sp, "print('123'") == 0);
	assert(sp_flush(&sp) == 0);
	assert(sp_sendc(&sp, ')') == 0);
	assert(sp_flush(&sp) == 0);
	assert(sp_sendc(&sp, '\n') == 0);
	assert(sp_recvline(&sp, buf, 100, false) == 4);
	buf[4] = 0;
	assert(strcmp(buf, "123\n") == 0);

	sp_free(&sp);
}

void test_io_interact(void)
{
	puts("\ntest_io_interact():");
	puts("===========================");
	puts("\ninteractive python shell:\n");

	subproc sp;
	char *argv[] = {"/usr/bin/env", "python3", NULL};
	assert(sp_open(&sp, argv[0], argv, NULL,
		(int[3]){SPIO_PTY, SPIO_PTY, SPIO_STDOUT},
		(size_t[3]){200, 300, 400}) == 0);

	// create some buffered data to ensure it's flushed correctly
	char recv_buf[1024] = {};
	assert(sp_recvuntil(&sp, recv_buf, sizeof(recv_buf), "Python", true) == 6);
	printf("%s", recv_buf);
	fflush(stdout);
	assert(sp_sends(&sp, "print('abcd')\n") == 0);
	
	sp_interact(&sp);
}

int main(void)
{
	test_signals();
	test_redirection();
	test_spio_options();
	test_errors();
	test_buffered_io(20);
	test_buffered_io(1);
	test_buffered_io(0);
	test_io_interact();
	return 0;
}
