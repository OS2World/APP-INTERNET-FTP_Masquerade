################################################################################
#
# Top-level make file for FTP Masquerade. This just sets the appropriate
# configuration file and calls the make file in the src directory.
#
#      Created: 18th June 2000
# Version 1.00: 18th June 2000
# Version 1.10: 17th January 2001
#
# (C) 2000-2001 Nicholas Paul Sheppard. See the file licence.txt for details.
#


default:
	@echo Please specify one of the following build targets:
	@echo linux, os2, solaris

linux:
	( cd src ; $(MAKE) all "CONFIG=../config/linux" )

os2:
	( cd src & $(MAKE) all "CONFIG=..\config\os2" )

solaris:
	( cd src ; $(MAKE) all "CONFIG=../config/solaris" )
