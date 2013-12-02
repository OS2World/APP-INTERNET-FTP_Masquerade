@echo off

setlocal
set PACKAGE=ftpm110s.zip

zip -9 -u %PACKAGE% src\*.c src\*.h src\Makefile
zip -9 -u %PACKAGE% doc\*.txt readme.txt
zip -9 -u %PACKAGE% Makefile config\*
zip -9 -u %PACKAGE% package package.cmd

endlocal