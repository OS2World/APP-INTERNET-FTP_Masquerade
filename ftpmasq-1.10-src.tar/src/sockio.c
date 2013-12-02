/*
 * sockio.c
 *
 * Utility routines for performing actions on sockets.
 *
 *      Created: 11th June 2000
 * Version 1.00: 6th November 2000
 * Version 1.10: 20th January 2001
 *
 * (C) 2000-2001 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "sockio.h"

#ifndef FIONREAD
# include <sys/stropts.h>
# define FIONREAD	I_NREAD
#endif


int sogets(int so, char *psz, int n)
/*
 * Read a line from a socket, a la fgets().
 *
 * so  - the socket to read from
 * psz - the buffer to read the line into
 * n   - the maximum length of the buffer
 *
 * Returns: -1, if there was an error (sets errno)
 *          the number of bytes read, otherwise
 */
{
	int	m;	/* number of bytes read this loop */
	int	t;	/* total number of bytes read, less EOL */
	int	bDone;	/* loop invariant */

	t = 0;
	bDone = 0;
	while ((t < (n-1)) && !bDone) {
		if ((m = read(so, psz, 1)) < 0) {
			return (-1);
		} else if ((m == 0) || (*psz == '\n')) {
			bDone = 1;
		} else if (*psz != '\r') {
			t++;
			psz++;
		}
	}
	*psz = '\0';

	return (t);
}


int sopipe(int so1, int so2)
/*
 * Copy data from one socket to another socket.
 *
 * soIn  - the socket to read from
 * soOut - the socket to write to
 *
 * Returns: -1, if there was an error (sets errno)
 *          the number of bytes copied, otherwise
 */
{
	char	ach[SOCKIO_MAXB];	/* buffer */
	int	n;			/* number of bytes left to send */
	int	t;			/* total number of bytes */
	int	m;			/* number of bytes read this loop */

	if (ioctl(so1, FIONREAD, &t) < 0)
		return (-1);

	n = t;
	while (n > 0) {
		if ((m = soread(so1, ach, sizeof(ach))) < 0)
			return (-1);
		else if (sowrite(so2, ach, m) < 0)
			return (-1);
		else
			n -= m;
	}

	return (t);
}


int soprintf(int so, const char *psz, ...)
/*
 * Write formatted data to a socket, a la fprintf()
 *
 * so  - the socket to write to
 * psz - the format string
 *
 * Returns: -1, if there was an error (sets errno)
 *          the number of bytes sent, otherwise
 */
{
	char	ach[SOCKIO_MAXB];	/* buffer */
	int	n;			/* return value */
	va_list	vp;			/* variable arguments */

	va_start(vp, psz);
	n = vsprintf(ach, psz, vp);
	va_end(vp);

	return (sowrite(so, ach, n));
}


int soread(int so, void *p, int n)
/*
 * Read data from a socket, a la fread().
 *
 * so - the socket to read from
 * p  - the buffer to read into
 * n  - the maximum length of the buffer
 *
 * Returns: -1, if there is an error
 *          the number of bytes read, otherwise
 */
{
	return (read(so, p, n));
}


int sowrite(int so, void *p, int n)
/*
 * Write data to a socket, a la fwrite().
 *
 * so - the socket to write to
 * p  - the buffer to write
 * n  - the number of bytes to write
 *
 * Returns: -1, if there is an error
 *          the number of bytes written, otherwise
 */
{
	int	m;	/* the number of bytes written this round */
	int	t;	/* the total number of bytes written */

	t = 0;
	while (t < n) {
		if ((m = write(so, p, n - t)) < 0)
			return (-1);
		else {
			t += m;
			p = (char *)p + m;
		}
	}

	return (t);
}
