FTP Masquerade

FTP Masquerade is a simple utility to permit active FTP sessions through a masquerading firewall. It works by fooling the FTP client into thinking the firewall is the FTP server, and the FTP server into thinking that the FTP requests are coming from the firewall.

I distribute FTP Masquerade as source code only. It uses strict ANSI C and BSD sockets, so, in principle, any compiler that supports these standards should be able to compile it (though you might need to muck around with the #includes a bit). I've tested it with GCC on Linux and OS/2 (configuration files for these two systems are included in the package); your mileage on other systems may vary. If there's anything reasonable I can do to make it compile out-of-the-box on other systems, I'm happy to do it, I just don't want the code to turn into an incomprehensible mass of #ifdefs to cater for every brain-damaged compiler and OS on Earth.

The latest version of FTP Masquerade is Version 1.10, released on 28th September 2001. This web site is currently the only official download point for FTP Masquerade; if anyone has suggestions for a better place I'm happy to hear about it.

    * ftpm110s.zip (53k) - source code in zip format (remember to use -d if using PKUNZIP), DOS-style.
    * ftpmasq-1.10-src.tar.gz (49k) - source code in gzipped tar format, UNIX-style 

Please contact me with any bugs, feature requests, compiler problems or suggestions, etc., and I'll see what I can do about them.
Known Bugs

None yet.
Last edited: 28th September 2001 
http://www.zeta.org.au/~nps/software/ftpmasq/en/index.html