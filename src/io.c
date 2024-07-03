#include "subproc/subproc.h"

int sp_sendc(subproc *sp, char c)
{
	struct sp_io_buffer *buf = &sp->buf[0];
	if (buf->start == NULL)	// unbuffered
	{
		if (write(sp->fds[0], &c, 1) != 1)
			seterror("write", return -1);
		return 0;
	}
	if (buf->ptr == buf->end)
		if (sp_flush(sp) == -1)
			return -1;
	*(buf->start++) = c;
}

int sp_sendn(subproc *sp, char *data, size_t n)
{
	
}

int sp_sends(subproc *sp, char *str)
{
	sp_sendn(sp, str, strlen(str));
}

int sp_flush(subproc *sp)
{

}

int sp_recvc(subproc *sp, bool from_strerr)
{

}

size_t sp_recvn(subproc *sp, char *data, size_t n, bool from_stderr)
{

}

size_t sp_recvuntil(subproc *sp, char *data, size_t size, char delim, bool from_stderr)
{

}

size_t sp_recvline(subproc *sp, char *data, size_t size, bool from_stderr)
{
	return sp_recvuntil(sp, data, size, '\n', from_stderr);
}

int sp_interact(subproc *sp)
{

}
