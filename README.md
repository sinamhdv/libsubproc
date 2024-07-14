# libsubproc

A subprocess interaction and IO library in C for Linux.

The interface and idea of this library is similar to the python `subprocess` library, or the IO parts of the `pwntools` library.

## Features

- TODO

## Example Usage

For more examples of different features look at the `examples/` directory or the `test/test.c` file.

The code below will spawn two subprocesses `cat` and `wc` and pipe the output of `cat` into `wc` so that it calls `cat <file> | wc -l` on its own source code. So it prints the number of lines in its own source. This example can also be built and run in `examples/`

```C
#include "subproc/subproc.h"

int main(void)
{
	subproc cat_sp, wc_sp;
	
	// open 'cat' and 'wc' subprocesses
	sp_open(&cat_sp, "/usr/bin/cat",
		(char*[3]){"/usr/bin/cat", __FILE__, NULL}, NULL,
		(int[3]){SPIO_PIPE, SPIO_PIPE, SPIO_DEVNULL},
		(size_t[3]){0, 0, 0});
	sp_open(&wc_sp, "/usr/bin/wc",
		(char*[3]){"/usr/bin/wc", "-l", NULL}, NULL,
		(int[3]){cat_sp.fds[1], SPIO_PIPE, SPIO_DEVNULL},
		(size_t[3]){0, 0, 0});
	
	// read the output from subprocesses and print it
	char output[256];
	ssize_t output_size = sp_recvline(&wc_sp, output, sizeof(output) - 1, false);
	output[output_size] = 0;
	int lines_number;
	sscanf(output, "%d", &lines_number);
	printf("%s has %d lines\n", __FILE__, lines_number);

	// terminate and free the subprocesses
	sp_free(&cat_sp);
	sp_free(&wc_sp);
	return 0;
}
```
