#ifndef SP_HEADER_IO_H
#define SP_HEADER_IO_H

#include "subproc/subproc.h"

/*
sp_io_buffer for subprocess stdin:

	[buf->start]...[buffered data not yet sent to subprocess]...[buf->ptr]...[free buffer space]...[buf->end]

sp_io_buffer for subprocess stdout/stderr:

	[buf->start]...[free buffer space]...[buf->ptr]...[buffered read data not yet passed to user]...[buf->end]
*/

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
ssize_t sp_recvn(subproc *sp, char *buf, size_t size, bool from_stderr);
ssize_t sp_recvuntil(subproc *sp, char *buf, size_t size, char *delim, bool from_stderr);
ssize_t sp_recvline(subproc *sp, char *buf, size_t size, bool from_stderr);
ssize_t sp_recvn_s(subproc *sp, char *buf, size_t size, bool from_stderr);
ssize_t sp_recvuntil_s(subproc *sp, char *buf, size_t size, char *delim, bool from_stderr);
ssize_t sp_recvline_s(subproc *sp, char *buf, size_t size, bool from_stderr);
int sp_interact(subproc *sp);

#endif	// SP_HEADER_IO_H
