# libsubproc

A library for C to spawn and interact with subprocesses on Linux

The interface and idea of this library is similar to the python `subprocess` library, or the IO parts of the `pwntools` library.

## Features


- Simple, easy-to-use, and documented interface
- Error handling and library-specific `errno` and `perror()`
- Custom IO buffering that can boost performance significantly and make things up to 80x faster. (see `examples/buffering_example.c` for a demo of the performance boost)
- Support for different ways of handling subprocess standard file descriptors, including pseudo-terminals, Linux pipes, redirection, piping into another subprocess, duplicating from parent, etc.

## Example

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

## Usage

- Run `make` in the project root to build the library.
- Use the include path of `$LIBROOT/include` and library path of `$LIBROOT/bin` to compile and link your program with `libsubproc`.
- Include `subproc/subproc.h` to get all of the necessary interface of the library.
