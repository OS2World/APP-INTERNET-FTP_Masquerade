################################################################################
#
# Make file for FTP Masquerade.
#
#      Created: 12th June 2000
# Version 1.00: 27th July 2000
#
# (C) 2000 Nicholas Paul Sheppard. See the file licence.txt for details.
#

include $(CONFIG)

OBJS = ftp$(O) ftpmasqd$(O) logio$(O) sockio$(O)

all: ftpmasqd$(X)

ftpmasqd$(X): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

%$(O): %.c
	$(CC) $(CFLAGS) -o $@ -c $<

ftp$(O): ftp.c ftp.h
ftpmasqd$(O): ftpmasqd.c ftp.h logio.h sockio.h
logio$(O): logio.c logio.h
sockio$(O): sockio.c sockio.h

clean:
	$(RM) $(OBJS)
