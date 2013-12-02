/*
 * ftpmasqd.c
 *
 * FTP masquerading daemon.
 *
 *      Created: 10th June 2000
 * Version 1.00: 27th July 2000
 * Version 1.10: 26th September 2001
 *
 * (C) 2000, 2001 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "ftp.h"
#include "logio.h"
#include "sockio.h"

#define FTPMASQ_PORT	8021
#define FTPMASQ_MAXL	256

/* linked list node definition */
typedef struct _achnode {
	char			ach[FTPMASQ_MAXL];			/* contents */
	struct _achnode *	next;					/* next node */
} achnode;

/* global variables */
FLOG *	flog;								/* log file */
char *	pszWelcomeFile;							/* welcome message file name */
char *	pszDefaultHost;							/* default host name */
int	bForceHost;							/* force use of pszDefaultHost? */


/* internal function prototypes */
int	masq_server(int);						/* masquerade a connection */
int	masq_listen(int, int);						/* set up for a data channel */
int	masq_accept(int, struct sockaddr_in *, int, int *, int *);	/* accept a data channel connection */


int main(int argc, char *argv[])
{
	int			soServer;	/* server socket to listen on */
	struct sockaddr_in	sinServer;	/* bind socket address */
	int			soClient;	/* socket connected to client */
	struct sockaddr_in	sinClient;	/* client address */
	int			bInetd;		/* being started by inetd? */
	int			i;		/* counter */
	int			a;		/* working variable */

	/* initialise variables */
	memset(&sinServer, 0, sizeof(sinServer));
	sinServer.sin_family = AF_INET;
	sinServer.sin_addr.s_addr = htonl(INADDR_ANY);
	sinServer.sin_port = htons(FTPMASQ_PORT);
	flog = NULL;
	pszWelcomeFile = NULL;
	pszDefaultHost = NULL;
	bInetd = 0;
	bForceHost = 0;

	/* parse command line */
	i = 1;
	while (i < argc) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'h':
					bForceHost = (argv[i][2] == '!');
					if (++i < argc) {
						pszDefaultHost = argv[i];
					} else {fprintf(stderr, "%s: required parameter missing\n", argv[0]);
						exit(1);
					}
					break;

				case 'm':
					if (++i < argc) {
						pszWelcomeFile = argv[i];
					} else {
        					fprintf(stderr, "%s: required parameter missing\n", argv[0]);
						exit(1);
					}
					break;

				case 'p':
					if (++i < argc) {
						a = atoi(argv[i]);
						if ((a < 0) || (a > 32267)) {
							fprintf(stderr, "%s: invalid port number %d\n", argv[0], a);
							exit(1);
						} else if (a == 0) {
							bInetd = 1;
						}
						sinServer.sin_port = htons(a);
					} else {
        					fprintf(stderr, "%s: required parameter missing\n", argv[0]);
						exit(1);
					}
					break;

				case 'v':
					if (++i < argc) {
						if (strcmp(argv[i], "-") == 0)
							flog = flogdup("ftpmasqd", stdout);
						else if ((flog = flogopen("ftpmasqd", argv[i])) == NULL) {
							fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
							exit(1);
						}
					} else {
						fprintf(stderr, "%s: required parameter missing\n", argv[0]);
						exit(1);
					}
					break;

				case '?':
					printf("FTP Masquerade 1.10  (C) 2000, 2001 Nicholas Paul Sheppard\n\n");
					printf("Valid options are:\n\n");
					printf("  -h[!] <host>  Set default host; ! connects to default host only\n");
					printf("  -m <file>     Send contents of <file> to client upon connection\n");
					printf("  -p <port>     Set port number (default %d)\n", FTPMASQ_PORT);
					printf("  -p 0          Start from inetd\n");
					printf("  -v <file>     Log to <file>; - for stdout\n");
					printf("  -?            Show this screen\n");
					exit(0);
					break;

				default:
					fprintf(stderr, "%s: unrecognised option `%s'\n", argv[0], argv[i]);
					break;
			}
		}
		i++;
	}

	if (!bInetd) {
		/* create a socket to listen to */
		if ((soServer = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			fprintf(stderr, "%s: socket(): %s\n", argv[0], strerror(errno));
			exit(1);
		}

		/* bind the socket to a local port */
		if (bind(soServer, (struct sockaddr *)&sinServer, sizeof(sinServer)) < 0) {
			fprintf(stderr, "%s: bind(): %s\n", argv[0], strerror(errno));
			close(soServer);
			exit(1);
		}

		/* listen for clients */
		if (listen(soServer, 5) < 0) {
			fprintf(stderr, "%s: listen(): %s\n", argv[0], strerror(errno));
			close(soServer);
			exit(1);
		}
		flogf(flog, "listening on port %d", ntohs(sinServer.sin_port));

		/* loop forever */
		for ( ; ; ) {
			a = sizeof(sinClient);
			if ((soClient = accept(soServer, (struct sockaddr *)&sinClient, &a)) < 0) {
				fprintf(stderr, "%s: accept(): %s\n", argv[0], strerror(errno));
				close(soServer);
				exit(1);
			}

			/* create a child to deal with the connection */
			if (fork() == 0) {
				/* we are the child */
				close(soServer);
				flogf(flog, "received connection from %s, port %d", inet_ntoa(sinClient.sin_addr), ntohs(sinClient.sin_port));
				i = masq_server(soClient);
				close(soClient);
				flogf(flog, "closed connection to %s, port %d", inet_ntoa(sinClient.sin_addr), ntohs(sinClient.sin_port));
				flogclose(flog);
				exit(i);
			} else {
				/* we are the parent */
				close(soClient);
			}
		}
	} else {
		/* serve the socket inherited from inetd */
		soClient = 0;
		a = sizeof(sinClient);
		getsockname(soClient, (struct sockaddr *)&sinClient, &a);
		flogf(flog, "received connection from %s, port %d", inet_ntoa(sinClient.sin_addr), ntohs(sinClient.sin_port));
		a = masq_server(soClient);
		flogf(flog, "closed connection to %s, port %d", inet_ntoa(sinClient.sin_addr), ntohs(sinClient.sin_port));
		flogclose(flog);
		exit(a);
	}

	/* unreachable code */
	return (0);
}

/* server messages */
char szGreeting[] =	"220 FTP Masquerade 1.10 ready.\r\n";
char szNoLogin[] =	"530 FTP Masquerade: Please login with [USER@]HOST and PASS.\r\n";
char szBadHost[] =	"531 FTP Masquerade: %s: host not found.\r\n";
char szStdError[] =	"421 FTP Masquerade: %s: %s. Closing connection.\r\n";
char szStdWarn[] =	"421 FTP Masquerade: %s: %s.\r\n";
char szBadPort[] =	"501 FTP Masquerade: Syntax error in parameters or arguments.\r\n";


int masq_server(int soClientControl)
/*
 * Masquerade an FTP connection.
 *
 * soClientControl	- the control channel connected to the client
 *
 * Returns: 0 on sucess
 *          1 on failure
 */
{
	int			soServerControl;	/* server control socket */
	int			soServerData;		/* server data socket */
	int			soClientData;		/* client data socket */
	int			soDataListen;		/* for listening for data connections */
	struct sockaddr_in	sinServer;		/* server address */
	struct sockaddr_in	sinClient;		/* client address */
	struct hostent *	h;			/* host entry structure */
	int			bPassive;		/* are we in passive mode? */
	char *			pch1;			/* working variable */
	char *			pch2;			/* working variable */
	int			bWelcome;		/* set to 1 if the server's welcome has been passed on */
	int			bDone;			/* set to 1 if finished */
	int			bAbort;			/* set to 1 if aborted */
	FILE *			f;			/* welcome message file */
	char			ach[FTPMASQ_MAXL];	/* buffer */
	char			szUser[FTPMASQ_MAXL];	/* user name */
	char			szHost[FTPMASQ_MAXL];	/* server host name */
	fd_set			fds;			/* file descriptor set for select() */
	int			n;			/* byte count */
	achnode *		head;			/* head of linked list */
	achnode *		tail;			/* tail of linked list */
	achnode *		p;			/* cursor in linked list */
	achnode *		temp;			/* temporary pointer */

	/* initialise variables */
	bPassive = 0;
	soServerControl = 0;
	soServerData = 0;
	soClientData = 0;
	soDataListen = 0;
	memset(&sinServer, 0, sizeof(sinServer));
	sinServer.sin_family = AF_INET;
	sinServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	sinServer.sin_port = htons(FTP_PORT);

	/* send the welcome message */
	if (pszWelcomeFile && ((f = fopen(pszWelcomeFile, "r")) != NULL)) {
		while (!feof(f)) {
			if (fgets(ach, sizeof(ach), f) == NULL)
				break;
			else
				soprintf(soClientControl, "220-%s", ach);
		}
		fclose(f);
	}

	/* send a greeting message */
	if (send(soClientControl, szGreeting, strlen(szGreeting), 0) < 0) {
		return(1);
	}

	/* get the "user" from the client */
	do {
		bDone = 1;
		if (sogets(soClientControl, ach, sizeof(ach)) < 0) {
			soprintf(soClientControl, szStdError, "recv()", strerror(errno));
			flogf(flog, "recv(): %s", strerror(errno));
			return (1);
		} else if (strncicmp(ach, "USER ", 5) != 0) {
			soprintf(soClientControl, szNoLogin);
			bDone = 0;
		}
	} while (!bDone);

	/* break it up into username, hostname and port */
	if ((pch1 = strchr(ach + 5, '@')) != NULL) {
		*pch1 = '\0';
		strcpy(szUser, ach + 5);
		pch1++;
	} else if (pszDefaultHost) {
		strcpy(szUser, ach + 5);
		pch1 = pszDefaultHost;
	} else {
		/* assume anonymous log-in */
		strcpy(szUser, "anonymous");
		pch1 = ach + 5;
	}
	if (bForceHost) {
		pch1 = pszDefaultHost;
	}
	if ((pch2 = strchr(pch1, ':')) != NULL) {
		if ((sinServer.sin_port = htons(atoi(pch2 + 1))) == 0)
			return (1);
		*pch2 = '\0';
	}
	strcpy(szHost, pch1);

	/* connect to the real server */
	flogf(flog, "connecting to %s on port %d.\n", szHost, ntohs(sinServer.sin_port));
	if ((sinServer.sin_addr.s_addr = inet_addr(szHost)) == -1) {
		if ((h = gethostbyname(szHost)) == NULL) {
			soprintf(soClientControl, szBadHost, szHost);
			flogf(flog, "host %s not found", szHost);
			return (1);
		} else {
			sinServer.sin_addr.s_addr = *(int *)(h->h_addr_list[0]);
		}
	}
	if ((soServerControl = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		soprintf(soClientControl, szStdError, "socket()", strerror(errno));
		flogf(flog, "socket(): %s", strerror(errno));
		return (1);
	}
	if (connect(soServerControl, (struct sockaddr *)&sinServer, sizeof(sinServer)) < 0) {
		soprintf(soServerControl, szStdError, "connect()", strerror(errno));
		flogf(flog, "connect(): %s", strerror(errno));
		return (1);
	}

	/* caputre the server's welcome message */
	bWelcome = 0;
	head = tail = NULL;
	do {
		if ((p = (achnode *)malloc(sizeof(achnode))) == NULL) {
			close(soServerControl);
			soprintf(soClientControl, szStdError, "malloc()", strerror(errno));
			flogf(flog, "malloc(): %s", strerror(errno));
			return (1);
		} else {
			sogets(soServerControl, p->ach, sizeof(p->ach));
			if (!head) {
				head = tail = p;
				strncpy(ach, p->ach, 3);
			} else {
				tail->next = p;
				tail = p;
			}
			bDone = !strncmp(ach, p->ach, 3) && (p->ach[3] != '-');
			p->next = NULL;
		}
	} while (!bDone);

	/* send the username to the real server */
	flogf(flog, "logging in as %s", szUser);
	if (soprintf(soServerControl, "USER %s\r\n", szUser, 0) < 0) {
		close(soServerControl);
		soprintf(soClientControl, szStdError, "send()", strerror(errno));
		flogf(flog, "send(): %s", strerror(errno));
		return (1);
	}

	/* main masquerading loop */
	bAbort = bDone = 0;
	while (!bDone && !bAbort) {
		/* wait for our sockets to say something */
		FD_ZERO(&fds);
		FD_SET(soClientControl, &fds);
		FD_SET(soServerControl, &fds);
		if (soServerData)
			FD_SET(soServerData, &fds);
		if (soClientData)
			FD_SET(soClientData, &fds);
		if (soDataListen)
			FD_SET(soDataListen, &fds);
		if (select(256, &fds, NULL, NULL, NULL) < 0) {
			soprintf(soClientControl, szStdError, "select()", strerror(errno));
			flogf(flog, "select(): %s", strerror(errno));
			bAbort = 1;
		}

		/* deal with sockets */
		if (FD_ISSET(soClientControl, &fds)) {
			/* client sent something on the control channel */
			if ((n = sogets(soClientControl, ach, sizeof(ach))) == 0) {
				/* client closed connection; stop */
				bDone = 1;
			} else if (strncicmp(ach, "PORT ", 5) == 0) {
				/* set up a data channel */
				if (soClientData)
					close(soClientData);
				if (soServerData)
					close(soServerData);
				if (ftp_port(ach + 5, &sinClient, sizeof(sinClient)) == -1) {
					soprintf(soClientControl, szBadPort);
				} else if ((soDataListen = masq_listen(soServerControl, 0)) < 0) {
					soprintf(soClientControl, szStdWarn, "masq_listen()", strerror(errno));
					flogf(flog, "masq_listen(): %s", strerror(errno));
					soDataListen = 0;
				}
			} else if (soprintf(soServerControl, "%s\r\n", ach) < 0) {
				soprintf(soClientControl, szStdError, "send()", strerror(errno));
				flogf(flog, "send(): %s", strerror(errno));
				bAbort = 1;
			}
		} else if (FD_ISSET(soServerControl, &fds)) {
			/* server sent something on the control channel */
			if ((n = sogets(soServerControl, ach, sizeof(ach))) == 0) {
				/* server closed connection; stop */
				bDone = 1;
			} else {
				if (!bWelcome) {
					/* pass on the welcome message */
					p = head;
					while (p) {
						memcpy(p->ach, ach, 3);
						p->ach[3] = '-';
						soprintf(soClientControl, "%s\r\n", p->ach);
						temp = p;
						p = p->next;
						free(temp);
					}
					bWelcome = 1;
				}
				if (strncmp(ach, "227", 3) == 0) {
					/* enter passive mode */
					if (soClientData)
						close(soClientData);
					if (soServerData)
						close(soServerData);
					if (ftp_port(strchr(ach, '(') + 1, &sinServer, sizeof(sinServer)) == -1) {
						soprintf(soClientControl, szBadPort);
					} else if ((soDataListen = masq_listen(soClientControl, 1)) < 0) {
						soprintf(soClientControl, szStdWarn, "masq_listen()", strerror(errno));
						flogf(flog, "masq_listen(): %s", strerror(errno));
						soDataListen = 0;
					} else {
						bPassive = 1;
					}
				}
				if (soprintf(soClientControl, "%s\r\n", ach) < 0) {
					bAbort = 1;
				}
			}
		} else if (soClientData && FD_ISSET(soClientData, &fds)) {
			/* client sent something on the data channel */
			if ((n = sopipe(soClientData, soServerData)) == 0) {
				/* client closed data connection */
				if (soServerData) {
					close(soServerData);
					soServerData = 0;
				}
				close(soClientData);
				soClientData = 0;
			} else if (n < 0) {
				soprintf(soClientControl, szStdError, "send()", strerror(errno));
				flogf(flog, "send(): %s", strerror(errno));
				bAbort = 1;
			}
		} else if (soServerData && FD_ISSET(soServerData, &fds)) {
			/* server sent something on the data channel */
			if ((n = sopipe(soServerData, soClientData)) == 0) {
				/* server closed data connection */
				if (soClientData) {
					close(soClientData);
					soClientData = 0;
				}
				close(soServerData);
				soServerData = 0;
			} else if (n < 0) {
				soprintf(soClientControl, szStdError, "send()", strerror(errno));
				flogf(flog, "send(): %s", strerror(errno));
				bAbort = 1;
			}
		} else if (soDataListen && FD_ISSET(soDataListen, &fds)) {
			/* someone connected to the data channel */
			if (bPassive) {
				n = masq_accept(soDataListen, &sinServer, sizeof(sinServer), &soServerData, &soClientData);
			} else {
				n = masq_accept(soDataListen, &sinClient, sizeof(sinClient), &soClientData, &soServerData);
			}
			if (n < 0) {
				soprintf(soClientControl, szStdWarn, "masq_accept()", strerror(errno));
				flogf(flog, "masq_accept(): %s", strerror(errno));
				soClientData = 0;
				soServerData = 0;
			}
			close(soDataListen);
			soDataListen = 0;
		}
	}

	/* clean up */
	if (soServerControl)
		close(soServerControl);
	if (soServerData)
		close(soServerData);
	if (soClientData)
		close(soClientData);
	if (soDataListen)
		close(soDataListen);

	return (bAbort);
}


int masq_listen(int soControl, int bPassive)
/*
 * Set up for a data channel.
 *
 * soControl - the control channel of the host we're listening for
 * bPassive  - are we in passive mode?
 *
 * Returns: -1 if there is an error (sets errno)
 *          the socket to listen on, otherwise
 */
{
	struct sockaddr_in	sin;			/* for bind() */
	struct sockaddr_in	sinLocal;		/* our address */
	int			ip[6];			/* bytewise break up of address/port */
	int			n;			/* working variable */
	int			soDataListen;		/* return value */

	/* create a socket to listen on */
	if ((soDataListen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return (-1);
	}

	/* set up a port for the remote host */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(0);
	if (bind(soDataListen, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		close(soDataListen);
		return (-1);
	}
	if (listen(soDataListen, 5) < 0) {
		close(soDataListen);
		return (-1);
	}

	/* break up the address into bytes */
	n = sizeof(sinLocal);
	getsockname(soControl, (struct sockaddr *)&sinLocal, &n);
	ip[0] = (ntohl(sinLocal.sin_addr.s_addr) & 0xff000000) >> 24;
	ip[1] = (ntohl(sinLocal.sin_addr.s_addr) & 0x00ff0000) >> 16;
	ip[2] = (ntohl(sinLocal.sin_addr.s_addr) & 0x0000ff00) >>  8;
	ip[3] = (ntohl(sinLocal.sin_addr.s_addr) & 0x000000ff)      ;
	n = sizeof(sin);
	getsockname(soDataListen, (struct sockaddr *)&sin, &n);
	ip[4] = (ntohs(sin.sin_port) & 0xff00) >> 8;
	ip[5] = (ntohs(sin.sin_port) & 0x00ff)     ;

	if (!bPassive) {
		/* pass the masqueraded port command to the server */
		if (soprintf(soControl, "PORT %d,%d,%d,%d,%d,%d\r\n", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5]) < 0) {
			close(soDataListen);
			return (-1);
		}
	} else {
		/* pass the masqueraded passive mode success to the client */
		if (soprintf(soControl, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5]) < 0) {
			close(soDataListen);
			return (-1);
		}
	}

	return (soDataListen);
}


int masq_accept(int soDataListen, struct sockaddr_in *psinTo, int nTo, int *psoDataTo, int *psoDataFrom)
/*
 * Accept a data channel connection.
 *
 * soDataListen - the socket we're listening on
 * psinTo       - the address of the host the connection is actually going to
 * nTo          - length of psin
 * psoDataTo    - socket that the connection is going to (output)
 * psoDataFrom  - socket that the connection is coming from (output)
 *
 * Returns: -1, if there is an error (sets errno)
 *          0, otherwise
 */
{
	struct sockaddr_in	sinFrom;	/* server address */
	int			nFrom;		/* size of sinServer */

	/* accept the connection */
	nFrom = sizeof(sinFrom);
	if ((*psoDataFrom = accept(soDataListen, (struct sockaddr *)&sinFrom, &nFrom)) < 0)
		return (-1);

	/* connect to the other host */
	if ((*psoDataTo = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		close(*psoDataFrom);
		return (-1);
	}
	if (connect(*psoDataTo, (struct sockaddr *)psinTo, nTo) < 0) {
		close(*psoDataFrom);
		close(*psoDataTo);
		return (-1);
	}

	return (0);
}
