/*
 * ftp.c
 *
 * Miscellaneous helper routines.
 *
 *      Created: 27th July 2000
 * Version 1.00: 27th July 2000
 *
 * (C) 2000 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftp.h"

int strncicmp(const char *psz1, const char *psz2, int n)
/*
 * Compare strings case-insensitively, for at most n characters. Some compilers
 * have strnicmp() or strncasecmp() for this, but we put it here so that
 * everyone can have it.
 *
 * psz1	- the first string to compare
 * psz2	- the second string to compare
 * n    - the maximum number of characters to compare
 *
 * Returns:	-1, if psz1 is lexicographically before psz2
 *		0, if psz1 and psz2 are equal
 *		1, if psz1 is lexicographically after psz2
 */
{
	const char *	pch1;	/* counter in psz1 */
	const char *	pch2;	/* counter in psz2 */

	pch1 = psz1;
	pch2 = psz2;
	while (n && toupper(*pch1) == toupper(*pch2) && (*pch1) && (*pch2)) {
		pch1++;
		pch2++;
		n--;
	}

	if (n) {
		if (toupper(*pch1) < toupper(*pch2))
			return (-1);
		else if (toupper(*pch1) > toupper(*pch2))
			return (1);
	}

	return (0);
}


int ftp_port(const char *psz, struct sockaddr_in *psin, int n)
/*
 * Parse a comma-separated list of bytes representing an address and port (as
 * for PORT and PASV commands).
 *
 * psz  - the string to parse
 * psin - the address specificed by psz (output)
 * n    - the length of psin
 *
 * Returns: 0 on success
 *          -1 if there was a syntax error
 */
{
	/* initialise simple psin fields */
	memset(psin, 0, n);
	psin->sin_family = AF_INET;

	/* skip white space */
	while (isspace((int)(*psz)))
		psz++;

	/* get host address */
	psin->sin_addr.s_addr = atoi(psz) << 24;
	if ((psz = strchr(psz, ',')) == NULL)
		return (-1);
	psz++;
	psin->sin_addr.s_addr |= atoi(psz) << 16;
	if ((psz = strchr(psz, ',')) == NULL)
		return (-1);
	psz++;
	psin->sin_addr.s_addr |= atoi(psz) << 8;
	if ((psz = strchr(psz, ',')) == NULL)
		return (-1);
	psz++;
	psin->sin_addr.s_addr |= atoi(psz);

	/* get the port */
	if ((psz = strchr(psz, ',')) == NULL)
		return (-1);
	psz++;
	psin->sin_port = atoi(psz) << 8;
	if ((psz = strchr(psz, ',')) == NULL)
		return (-1);
	psz++;
	psin->sin_port |= atoi(psz);

	/* convert number to network order */
	psin->sin_addr.s_addr = htonl(psin->sin_addr.s_addr);
	psin->sin_port = htons(psin->sin_port);

	return (0);
}
