#include "subproc/subproc.h"

static bool send_all_unbuffered(int fd, void *data, size_t n)
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

int sp_sendc(subproc *sp, unsigned char c)
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
	*(buf->ptr++) = c;
	return 0;
}

int sp_sendn(subproc *sp, void *data, size_t size)
{
	struct sp_io_buffer *buf = &sp->buf[0];
	size_t remaining_bufsize = (size_t)(buf->end) - (size_t)(buf->ptr);
	if (buf->start == NULL || remaining_bufsize < size)	// direct write syscall
	{
		if (buf->start != NULL)	// flush any remaining data in buf
			if (sp_flush(sp) == -1)
				return -1;
		return send_all_unbuffered(sp->fds[0], data, size) ? 0 : -1;
	}
	memcpy(buf->ptr, data, size);
	buf->ptr += size;
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
	if (buf->start == NULL)	// unbuffered
	{
		unsigned char c;
		if (read(fd, &c, 1) <= 0)
			seterror("read", return -1);
		return c;
	}
	if (buf->ptr != buf->end)	// already have data in buffer
		return *buf->ptr++;
	ssize_t result = refill_recv_buffer(buf, fd);
	if (result == -1 || result == 0)
		return -1;
	DBGCHECK(buf->ptr != buf->end);
	return *buf->ptr++;
}

static ssize_t internal_buffered_recv_array(
	struct sp_io_buffer *buf,
	int fd,
	void *data,
	size_t size)
{
	size_t read_size = 0;
	while (read_size < size)
	{
		size_t buf_content_size = (size_t)buf->end - (size_t)buf->ptr;
		if (size - read_size <= buf_content_size)	// the data left in buffer is enough
		{
			memcpy(data + read_size, buf->ptr, size - read_size);
			buf->ptr += size - read_size;
			read_size = size;
			return read_size;
		}
		memcpy(data + read_size, buf->ptr, buf_content_size);
		read_size += buf_content_size;
		buf->ptr = buf->end;
		ssize_t refill_result = refill_recv_buffer(buf, fd);
		if (refill_result == -1)
			return -1;
		else if (refill_result == 0)
			break;
	}
	return read_size;
}

ssize_t sp_recvn(subproc *sp, void *data, size_t size, bool from_stderr)
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
	return internal_buffered_recv_array(buf, fd, data, size);
}

ssize_t sp_recvuntil(subproc *sp, void *data, size_t size, char *delim, bool from_stderr)
{
	size_t delim_size = strlen(delim);
	size_t read_size;
	for (read_size = 0; read_size < size; read_size++)
	{
		if (read_size >= delim_size && strncmp((char *)data + read_size - delim_size, delim, delim_size) == 0)
			return read_size;
		int read_char = sp_recvc(sp, from_stderr);
		if (read_char == -1)
			return read_size;
		((unsigned char *)data)[read_size] = (unsigned char)read_char;
	}
	return read_size;
}

ssize_t sp_recvline(subproc *sp, void *data, size_t size, bool from_stderr)
{
	return sp_recvuntil(sp, data, size, "\n", from_stderr);
}

int sp_interact(subproc *sp)
{
	if (sp_flush(sp) == -1) return -1;
	struct pollfd poll_fds[3];
	poll_fds[0].fd = 0;
	poll_fds[0].events = POLL_IN;
	for (int i = 1; i <= 2; i++)
	{
		if (sp->buf[i].ptr != sp->buf[i].end)	// print all buffered output
		{
			if (!send_all_unbuffered(1, sp->buf[i].ptr, (size_t)sp->buf[i].end - (size_t)sp->buf[i].ptr))
				return -1;
			sp->buf[i].ptr = sp->buf[i].end;
		}
		if (sp->fds[i] != -1)
		{
			poll_fds[i].fd = sp->fds[i];
			poll_fds[i].events = POLL_IN;
		}
		else
			poll_fds[i].fd = -1;
	}
	if (poll_fds[1].fd == poll_fds[2].fd)
		poll_fds[2].fd = -1;
	while (true)
	{
		int result = poll(poll_fds, sizeof(poll_fds)/sizeof(poll_fds[0]), -1);
		if (result == -1)
			seterror("poll", return -1);
		for (int i = 0; i < sizeof(poll_fds)/sizeof(poll_fds[0]); i++)
		{
			if (poll_fds[i].fd != -1 && poll_fds[i].revents != 0)
			{
				if (poll_fds[i].revents & POLL_IN)	// data available to read
				{
					char read_buf[1024];
					ssize_t read_size = read(poll_fds[i].fd, read_buf, sizeof(read_buf));
					if (read_size == -1) seterror("read", return -1);
					if (poll_fds[i].fd == 0)	// read from user => send to subprocess
					{
						if (!send_all_unbuffered(sp->fds[0], read_buf, read_size))
						{
							puts("[!] Error: could not send entered data to subprocess");
							fflush(stdout);
						}
					}
					else	// read from subprocess => print for user
					{
						send_all_unbuffered(1, read_buf, read_size);
					}
				}
				else	// POLL_ERR or POLL_HUP
				{
					// error reading data from user => continue printing subprocess output

					if (poll_fds[i].fd != 0)	// error reading data from subprocess => let the user know
					{
						puts("[!] Stream closed");
						fflush(stdout);
						return 0;
					}
				}
			}
		}
	}
}
