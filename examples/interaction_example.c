/*
example program that interacts with a python3 shell opened as a subprocess
*/
#include "subproc/subproc.h"

#define GREEN "\033[32m"
#define RESET "\033[0m"

int main(void)
{
	subproc sp;
	char output_buf[1024];
	ssize_t read_size;

	// open the subprocess
	sp_open(&sp, "/usr/bin/env",
		(char*[3]){"/usr/bin/env", "python3", NULL}, NULL,
		(int[3]){SPIO_PTY, SPIO_PTY, SPIO_PTY},
		(size_t[3]){256, 256, 256});

	// read prompt
	read_size = sp_recvuntil(&sp, output_buf, sizeof(output_buf) - 1, ">>> ", false);
	output_buf[read_size] = 0;
	printf(GREEN "Read initial message and prompt:" RESET "\n%s\n", output_buf);

	// send 'x = 2 + 5'
	sp_sends(&sp, "x = 2 + 5\n");
	printf(GREEN "sent: " RESET "x = 2 + 5\n");
	
	// read prompt
	read_size = sp_recvuntil(&sp, output_buf, sizeof(output_buf) - 1, ">>> ", false);
	output_buf[read_size] = 0;
	printf(GREEN "received new prompt line: " RESET "'%s'\n", output_buf);

	// send 'print("test message from python with a number %d" % x)'
	sp_sends(&sp, "print(\"test message from python with a number %d\" % x)\n");
	printf(GREEN "sent: " RESET "print(\"test message from python with a number %%d\" %% x)\n");

	// receive response
	read_size = sp_recvline(&sp, output_buf, sizeof(output_buf) - 1, false);
	output_buf[read_size] = 0;
	output_buf[strcspn(output_buf, "\n")] = 0;
	printf(GREEN "received response from subprocess: " RESET "'%s'\n", output_buf);

	// start interactive shell
	puts(GREEN "\nNow calling sp_interact() to start an interactive python shell!" RESET);
	sp_interact(&sp);

	// print return code and terminate and free the subprocess
	sp_wait(&sp, 0);
	printf(GREEN "subprocess ended with return code: %d\n" RESET, sp.returncode);
	sp_free(&sp);
}