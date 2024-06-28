#ifndef SP_HEADER_ERRORS_H
#define SP_HEADER_ERRORS_H

extern int sp_errno;	// last library errno we faced in sp functions
extern char *sp_errfunc;	// name of the library function that caused this errno

void sp_perror(char *msg);

#endif	// SP_HEADER_ERRORS_H