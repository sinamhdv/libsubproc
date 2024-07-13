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
	struct sp_io_buffer *buf = &sp->buf[from_strerr ? 2 : 1];
	int fd = sp->fds[from_strerr ? 2 : 1];
	if (sp_flush(sp) == -1)
		return -1;
	unsigned char c;
	if (buf->start == NULL)	// unbuffered
	{
		if (read(fd, &c, 1) <= 0)
			return -1;
		return c;
	}
	if (buf->ptr != buf->end)
		return *buf->ptr++;
	size_t bufsize = (size_t)buf->end - (size_t)buf->start;
	size_t read_size = read(fd, buf->start, bufsize);
	if (read_size == bufsize)
		buf->ptr = buf->start;
	else
	{
		if ((ssize_t)read_size <= -1)
			return -1;
		size_t diff_size = bufsize - read_size;
		memmove(buf->start + diff_size, buf->start, read_size);
		buf->ptr = buf->start + diff_size;
	}
	return *buf->ptr++;
}

ssize_t sp_recvn(subproc *sp, char *data, size_t n, bool from_stderr)
{

}

ssize_t sp_recvuntil(subproc *sp, char *data, size_t size, char delim, bool from_stderr)
{

}

ssize_t sp_recvline(subproc *sp, char *data, size_t size, bool from_stderr)
{
	return sp_recvuntil(sp, data, size, '\n', from_stderr);
}

int sp_interact(subproc *sp)
{

}
