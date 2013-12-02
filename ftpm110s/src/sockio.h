/*
 * sockio.h
 *
 * Utility routines for performing actions on sockets.
 *
 *      Created: 11th June 2000
 * Version 1.00: 18th June 2000
 *
 * (C) 2000 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#ifndef _NPS_SOCKIO_H
#define _NPS_SOCKIO_H

/* buffer size */
#define SOCKIO_MAXB	256

/* routines in sockio.c */
int	sogets(int, char *, int);		/* read a line from a socket, a la fgets() */
int	sopipe(int, int);			/* transfer data between two sockets */
int	soprintf(int, const char *, ...);	/* write formatted to a socket, a la fprintf() */
int	soread(int, void *, int);		/* read data from a socket, a la fread() */
int	sowrite(int, void *, int);		/* write data to a socket, a la fwrite() */

#endif
