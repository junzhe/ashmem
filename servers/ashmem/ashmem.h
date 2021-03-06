#ifndef _ASHMEM_H
#define _AHSMEM_H

#define _POSIX_SOURCE      1	/* tell headers to include POSIX stuff */
#define _MINIX             1	/* tell headers to include MINIX stuff */
#define _SYSTEM            1    /* get OK and negative error codes */

#define ASHMEM_NAME_LEN 256
#define ASHMEM_MAJOR 30

#define ASHMEM_NOT_PURGED 0
#define ASHMEM_IS_PURGED 1

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

int do_ashmem_create_region(message *m);
int do_ashmem_mmap_region(message *m);
int do_ashmem_release_region(message *m);
int do_ashmem_set_name_region(message *m);
int do_ashmem_set_size_region(message *m);
int do_ashmem_set_prot_region(message *m);
int do_ashmem_pin_region(message *m);
int do_ashmem_unpin_region(message *m);
int do_ashmem_get_size_region(message *m);

#endif /* _ASHMEM_H */
