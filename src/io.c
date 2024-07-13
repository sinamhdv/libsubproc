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

static ssize_t refill_recv_buffer(struct sp_io_buffer *buf, int fd)
{
	size_t bufsize = (size_t)buf->end - (size_t)buf->start;
	size_t read_size = read(fd, buf->start, bufsize);
	if (read_size == (size_t)-1)
		seterror("read", return -1);
	buf->ptr = buf->start;
	if (read_size < bufsize)
	{
		size_t diff_size = bufsize - read_size;
		memmove(buf->start + diff_size, buf->start, read_size);
		buf->ptr = buf->start + diff_size;
	}
	return read_size;
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
			seterror("read", return -1);
		return c;
	}
	if (buf->ptr != buf->end)	// already have data in buffer
		return *buf->ptr++;
	ssize_t result = refill_recv_buffer(buf, fd);
	if (result == -1)
		return -1;
	return *buf->ptr++;
}

static ssize_t internal_buffered_recv_string(
	struct sp_io_buffer *buf,
	int fd,
	char *data,
	size_t size,
	bool has_delim,
	char delim)
{
	size_t read_size = 0;
	bool eof_reached = false;
	while (read_size < size)
	{
		size_t full_buf_size = (size_t)buf->end - (size_t)buf->start;
		size_t buf_content_size = (size_t)buf->end - (size_t)buf->ptr;
		char *found_ptr;
		if (has_delim && (found_ptr = memchr(buf->ptr, delim, buf_content_size)) != NULL)	// delim found
		{
			size_t size_to_delim = ((size_t)found_ptr - (size_t)buf->ptr) + 1;
			if (size - read_size >= size_to_delim)
			{
				memcpy(data + read_size, buf->ptr, size_to_delim);
				buf->ptr = found_ptr + 1;
				read_size += size_to_delim;
				return read_size;
			}
		}
		if (size - read_size <= buf_content_size)	// the data left in buffer is enough
		{
			memcpy(data + read_size, buf->ptr, size - read_size);
			buf->ptr += size - read_size;
			return read_size;
		}
		memcpy(data + read_size, buf->ptr, buf_content_size);
		read_size += buf_content_size;
		buf->ptr = buf->end;
		if (eof_reached)
			return read_size;
		ssize_t refill_result = refill_recv_buffer(buf, fd);
		if (refill_result == -1)
			return -1;
		if (refill_result < full_buf_size)
			eof_reached = true;
	}
	return read_size;
}

ssize_t sp_recvn(subproc *sp, char *data, size_t size, bool from_stderr)
{
	struct sp_io_buffer *buf = &sp->buf[from_stderr ? 2 : 1];
	int fd = sp->fds[from_stderr ? 2 : 1];
	if (sp_flush(sp) == -1)
		return -1;
	if (buf->start == NULL)	// unbuffered
	{
		ssize_t ret = read(fd, data, size);
		if (ret < 0) seterror("read", return -1);
		return ret;
	}
	return internal_buffered_recv_string(buf, fd, data, size, false, 0);
}

ssize_t sp_recvuntil(subproc *sp, char *data, size_t size, char delim, bool from_stderr)
{
	struct sp_io_buffer *buf = &sp->buf[from_stderr ? 2 : 1];
	int fd = sp->fds[from_stderr ? 2 : 1];
	if (sp_flush(sp) == -1)
		return -1;
	if (buf->start == NULL)	// unbuffered
	{
		unsigned char c = delim == 0 ? 1 : 0;
		size_t read_size = 0;
		while (c != delim && read_size < size)
		{
			size_t result = read(fd, &c, 1);
			if (result == (size_t)-1)
				seterror("read", return -1);
			if (result == 0)
				break;
			data[read_size++] = c;
		}
		return read_size;
	}
	return internal_buffered_recv_string(buf, fd, data, size, true, delim);
}

ssize_t sp_recvline(subproc *sp, char *data, size_t size, bool from_stderr)
{
	return sp_recvuntil(sp, data, size, '\n', from_stderr);
}

int sp_interact(subproc *sp)
{

}
