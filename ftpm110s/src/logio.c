/*
 * logio.c
 *
 * Utility routines for maintaing log files.
 *
 *      Created: 18th June 2000
 * Version 1.00: 18th June 2000
 *
 * (C) 2000 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "logio.h"

int flogclose(FLOG *flog)
/*
 * Close a log file.
 *
 * flog - the log file to be closed (NULL to do nothing)
 *
 * Returns: EOF on error
 *          0, otherwise
 */
{
	int	n;	/* return value */

	if (flog) {
		n = fclose(flog->f);
		free(flog->psz);
		free(flog);
	} else {
		n = 0;
	}

	return (n);
}


FLOG *flogdup(const char *psz, FILE *f)
/*
 * Start a log on a file that has already been opened.
 *
 * psz - the process' name
 * f   - the file to log to
 *
 * Returns: NULL, if there is an error (sets errno)
 *          (a pointer to) a new FLOG structure, otherwise
 */
{
	FLOG *	flog;	/* return value */

	/* allocate memory */
	if ((flog = (FLOG *)malloc(sizeof(FLOG))) == NULL) {
		errno = ENOMEM;
		return (NULL);
	} else if ((flog->psz = (char *)malloc(strlen(psz)+1)) == NULL) {
			free(flog);
			errno = ENOMEM;
			return (NULL);
	}

	/* fill the fields */
	flog->f = f;
	strcpy(flog->psz, psz);

	return (flog);
}


int flogf(FLOG *flog, const char *psz, ...)
/*
 * Add a line to a log file.
 *
 * flog - the log file to write to (NULL to do nothing)
 * psz  - the format string
 *
 * Returns: the number of characters written to the log file
 */
{
	int		n;	/* return value */
	time_t		t;	/* seconds since 1970 */
	struct tm *	ptm;	/* current local time */
	char		sz[26];	/* current time in asctime() form */
	va_list		vp;	/* variable arguments */

	if (flog) {
		/* print the process idenification */
		time(&t);
		ptm = localtime(&t);
		strcpy(sz, asctime(ptm));
		sz[24] = 0;
		n = fprintf(flog->f, "%s %s[%d]: ", sz, flog->psz, getpid());

		/* print the message */
		va_start(vp, psz);
		n += vfprintf(flog->f, psz, vp);
		va_end(vp);

		/* print a new line */
		n += fprintf(flog->f, "\n");

		fflush(flog->f);
	} else {
		n = 0;
	}

	return (n);
}


FLOG *flogopen(const char *pszProcess, const char *pszFile)
/*
 * Open a log file. The log file will be opened for appending if it already
 * exists.
 *
 * pszProcess - the name of the process
 * pszFile    - the name of the log file
 *
 * Returns: NULL, if there was an error (sets errno)
 *          (a pointer to) a new FLOG structure, otherwise
 */
{
	FILE *	f;	/* the file */

	/* open the file */
	if ((f = fopen(pszFile, "a")) == NULL)
		return (NULL);

	/* use flogdup() to do all the work */
	return (flogdup(pszProcess, f));
}
