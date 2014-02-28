# Makefile for IPC server
PROG= 	ashmem	
SRCS=   ashmem.c	

FILES=$(PROG).conf
FILESNAME=$(PROG)
FILESDIR= /etc/system.conf.d

DPADD+= ${LIBCHARDRIVER} ${LIBSYS}
LDADD+= -lchardriver -lsys

MAN=

BINDIR?= /usr/sbin

.include <minix.service.mk>
