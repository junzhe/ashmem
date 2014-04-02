# Makefile for IPC server
PROG= 	zygote	
SRCS=   zygote.c	

FILES=$(PROG).conf
FILESNAME=$(PROG)
FILESDIR= /etc/system.conf.d

DPADD+=  ${LIBSYS}
LDADD+=  -lsys

MAN=

BINDIR?= /usr/sbin

.include <minix.service.mk>
