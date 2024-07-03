#ifndef SP_HEADER_IO_H
#define SP_HEADER_IO_H

#include "subproc/subproc.h"

struct sp_io_buffer
{
	char *start;
	char *ptr;
	char *end;
};

typedef struct subproc subproc;

int sp_sendc(subproc *sp, char c);
int sp_sendn(subproc *sp, char *data, size_t n);
int sp_sends(subproc *sp, char *str);
int sp_flush(subproc *sp);
int sp_recvc(subproc *sp, bool from_stderr);
size_t sp_recvn(subproc *sp, char *buf, size_t n, bool from_stderr);
size_t sp_recvuntil(subproc *sp, char *buf, size_t bufsize, char delim, bool from_stderr);
size_t sp_recvline(subproc *sp, char *buf, size_t bufsize, bool from_stderr);
int sp_interact(subproc *sp);

#endif	// SP_HEADER_IO_H
