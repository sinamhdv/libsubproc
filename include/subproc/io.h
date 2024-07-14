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
	unsigned char *start;
	unsigned char *ptr;
	unsigned char *end;
};

typedef struct subproc subproc;

/**
 * @brief send a character to the subprocess
 * 
 * @param sp 
 * @param c the character to send
 * @return 0 on success, -1 on error
 */
int sp_sendc(subproc *sp, unsigned char c);

/**
 * @brief send an array of bytes to the subprocess
 * 
 * @param sp 
 * @param data the data buffer
 * @param size size of data in the data buffer
 * @return 0 on success, -1 on error
 */
int sp_sendn(subproc *sp, void *data, size_t size);

/**
 * @brief send a null-terminated string to the subprocess
 * 
 * @param sp 
 * @param str the string to send
 * @return 0 on success, -1 on error
 */
int sp_sends(subproc *sp, char *str);

/**
 * @brief flush the input that is buffered and not sent to the subprocess yet
 * 
 * @param sp 
 * @return 0 on success, if no buffered data left to flush, or if the subprocess in unbuffered. -1 on error
 */
int sp_flush(subproc *sp);

/**
 * @brief receive a character from the subprocess
 * 
 * @param sp 
 * @param from_stderr read from stderr instead of stdout
 * @return -1 on error or EOF. the 1-byte character received on success
 */
int sp_recvc(subproc *sp, bool from_stderr);

/**
 * @brief receive an array of bytes from the subprocess
 * 
 * @param sp 
 * @param buf output buffer to write to
 * @param size size of output buffer
 * @param from_stderr read from stderr instead of stdout
 * @return size of data received (0 on EOF), or -1 on error.
 */
ssize_t sp_recvn(subproc *sp, void *data, size_t size, bool from_stderr);

/**
 * @brief receive data from the subprocess until a delimiter string
 *
 * @note if the delimiter is not found before the first 'size' bytes or EOF, returns number of bytes read and stops
 *
 * @param sp 
 * @param buf output buffer
 * @param size max size of output buffer
 * @param delim null-terminated delimiter string to read until
 * @param from_stderr read from stderr instead of stdout
 * @return size of data received (0 on EOF), or -1 on error.
 */
ssize_t sp_recvuntil(subproc *sp, void *data, size_t size, char *delim, bool from_stderr);

/**
 * @brief receive a line of data from the subprocess (until '\n')
 * 
 * @note if '\n' is not found before the first 'size' bytes or EOF, returns the number of bytes read and stops
 *
 * @param sp 
 * @param buf output buffer
 * @param size maximum size of output buffer
 * @param from_stderr read from stderr instead of stdout
 * @return size of data received (0 on EOF), or -1 on error
 */
ssize_t sp_recvline(subproc *sp, void *data, size_t size, bool from_stderr);

/**
 * @brief start an interactive prompt between the parent and subprocess
 *
 * @param sp 
 * @return -1 on error, 0 on success or the end of output stream from the subprocess
 */
int sp_interact(subproc *sp);

#endif	// SP_HEADER_IO_H
