#include "subproc/subproc.h"

static bool send_all_unbuffered(int fd, char *data, size_t n)
{
	size_t sent = 0;
	while (sent < n)
	{
		size_t ret = write(fd, data, n - sent);
		if (ret == 0 || ret == -1)
			seterror("write", return false);
		sent += ret;
	}
	return true;
}

int sp_sendc(subproc *sp, char c)
{
	struct sp_io_buffer *buf = &sp->buf[0];
	if (buf->start == NULL)	// unbuffered
	{
		if (write(sp->fds[0], &c, 1) != 1)
			seterror("write", return -1);
		return 0;
	}
	if (buf->ptr == buf->end)	// flush if the buffer is full
		if (sp_flush(sp) == -1)
			return -1;
	*(buf->start++) = c;
	return 0;
}

int sp_sendn(subproc *sp, char *data, size_t n)
{
	struct sp_io_buffer *buf = &sp->buf[0];
	size_t remaining_bufsize = (size_t)(buf->end) - (size_t)(buf->ptr);
	if (buf->start == NULL || remaining_bufsize < n)	// direct write syscall
	{
		if (buf->start != NULL)	// flush any remaining data in buf
			if (sp_flush(sp) == -1)
				return -1;
		return send_all_unbuffered(sp->fds[0], data, n) ? 0 : -1;
	}
	memcpy(buf->ptr, data, n);
	buf->ptr += n;
	return 0;
}

int sp_sends(subproc *sp, char *str)
{
	return sp_sendn(sp, str, strlen(str));
}

int sp_flush(subproc *sp)
{
	struct sp_io_buffer *buf = &sp->buf[0];
	if (buf->start == NULL || buf->ptr == buf->start) return 0;
	size_t data_size = (size_t)(buf->ptr) - (size_t)(buf->start);
	if (!send_all_unbuffered(sp->fds[0], buf->start, data_size))
		return -1;
	buf->ptr = buf->start;
	return 0;
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
