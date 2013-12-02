/*
 * ftp.h
 *
 * Header file for miscellaneous utility routines.
 *
 *      Created: 27th July 2000
 * Version 1.00: 27th July 2000
 *
 * (C) 2000 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#ifndef _NPS_FTP_H
#define _NPS_FTP_H

#include <netinet/in.h>

#define FTP_PORT	21

int	strncicmp(const char *, const char *, int);		/* case-insensitve strncmp() */
int	ftp_port(const char *, struct sockaddr_in *, int);	/* parse addresses and ports */

#endif
