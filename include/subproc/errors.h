#ifndef SP_HEADER_ERRORS_H
#define SP_HEADER_ERRORS_H

#define seterror(errfunc, action) { \
			sp_errno = errno; \
			sp_errfunc = (errfunc); \
			action; }

extern int sp_errno;	// last library errno we faced in sp functions
extern char *sp_errfunc;	// name of the library function that caused this errno

/**
 * @brief print the error message for the latest libsubproc error to stderr
 * 
 * @param msg optional message to display before the error, or NULL for no message
 */
void sp_perror(char *msg);

#endif	// SP_HEADER_ERRORS_H