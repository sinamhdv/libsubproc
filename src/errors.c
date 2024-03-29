#include "subproc/subproc.h"

int sp_errno;
char *sp_errfunc;

void sp_perror(char *msg)
{
	char buf[256];
	buf[0] = 0;
	char *errbuf = strerror_r(sp_errno, buf, sizeof(buf));
	if (msg == NULL)
		fprintf(stderr, "%s: %s\n", sp_errfunc, errbuf);
	else
		fprintf(stderr, "%s: %s: %s\n", msg, sp_errfunc, errbuf);
}
