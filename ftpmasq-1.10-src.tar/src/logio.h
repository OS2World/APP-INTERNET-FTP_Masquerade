/*
 * logio.h
 *
 * Utility routines for maintaing log files.
 *
 *      Created: 18th June 2000
 * Version 1.00: 18th June 2000
 *
 * (C) 2000 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#ifndef _NPS_LOGIO_H
#define _NPS_LOGIO_H

#include <stdio.h>

/* type definition for a log file */
typedef struct _FLOG {
	FILE *	f;		/* file pointer */
	char *	psz;		/* process name */
} FLOG;


/* routines in logio.c */
int	flogclose(FLOG *);				/* close a log file */
FLOG *	flogdup(const char *, FILE *);			/* create a log on an existing file stream */
int	flogf(FLOG *, const char *, ...);		/* add a line to a log file */
FLOG *	flogopen(const char *, const char *);		/* open a log file */

#endif
