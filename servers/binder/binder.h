#ifndef _BINDER_H
#define _BINDER_H
#define _POSIX_SOURCE      1    /* tell headers to include POSIX stuff */
#define _MINIX             1    /* tell headers to include MINIX stuff */
#define _SYSTEM            1    /* get OK and negative error codes */

#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/ipc.h>
#include <minix/endpoint.h>
#include <minix/sysutil.h>
#include <minix/const.h>
#include <minix/type.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/safecopies.h>
#include <minix/chardriver.h>
#include <minix/ds.h>
#include <minix/list.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/rbtree.h>
#include <sys/mman.h>
#include <sys/ioc_ashmem.h>
#include <machine/vm.h>
#include <machine/vmparam.h>
#include <sys/vm.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <lib.h>

int do_binder_create(message *);
int do_binder_read(message *);
int do_binder_write(message *);
int do_binder_read_kernel(message *);
int do_binder_write_kernel(message *);

#endif /* _BINDER_H */

