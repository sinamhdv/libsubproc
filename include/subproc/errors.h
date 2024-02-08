#ifndef SP_HEADER_ERRORS_H
#define SP_HEADER_ERRORS_H

extern int sp_errno;	// last library errno we faced in sp functions
extern char *sp_errfunc;	// name of the library function that caused this errno

#endif	// HEADER_SP_ERRORS_H