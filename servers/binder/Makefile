# Makefile for IPC server
PROG= 	binder	
SRCS=   binder.c	

FILES=$(PROG).conf
FILESNAME=$(PROG)
FILESDIR= /etc/system.conf.d

DPADD+=  ${LIBSYS}
LDADD+=  -lsys

MAN=

BINDIR?= /usr/sbin

.include <minix.service.mk>
