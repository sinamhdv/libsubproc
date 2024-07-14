/*
An example to calculate the performance boost of libsubproc IO buffering
*/
#include "subproc/subproc.h"
#include <time.h>
#include <assert.h>

#define IO_DATA_SIZE 1048576	// 1MB
#define BUFFER_SIZE 4096	// 4KB

#define STRINGIZE(x) STRINGIZE_INTERNAL(x)
#define STRINGIZE_INTERNAL(x) #x

void do_heavy_io(size_t bufsize)
{
	subproc sp;

	// read a lot of data from /dev/urandom
	char *argv[] = {"/usr/bin/head", "-c", STRINGIZE(IO_DATA_SIZE), "/dev/urandom", NULL};

	// open subprocess
	assert(sp_open(&sp, argv[0], argv, NULL,
		(int[3]){SPIO_PTY, SPIO_PTY, SPIO_PTY},
		(size_t[3]){bufsize, bufsize, bufsize}) == 0);
	
	for (size_t i = 0; i < IO_DATA_SIZE; i++)
	{
		if (sp_recvc(&sp, false) == -1)
		{
			sp_perror("Error in recvc");
			printf("i = %lu\n", i);
			break;
		}
	}

	sp_free(&sp);
}

double measure_time(void (*func)(size_t), size_t arg)
{
	clock_t start_time = clock();
	func(arg);
	clock_t end_time = clock();
	double duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
	return duration;
}

int main(void)
{
	puts("Doing buffered IO...");
	double buffered_time = measure_time(do_heavy_io, BUFFER_SIZE);
	puts("Doing unbuffered IO...");
	double unbuffered_time = measure_time(do_heavy_io, 0);

	printf("Buffered IO time: %lfms\n", buffered_time);
	printf("Unbuffered IO time: %lfms\n", unbuffered_time);

	double boost = (unbuffered_time / buffered_time) * 100.0 - 100.0;
	printf("Buffering performance boost: %lf%%\n", boost);
	return 0;
}