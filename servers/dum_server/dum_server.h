#ifndef _DUM_SERVER_H
#define _DUM_SERVER_H

#define _POSIX_SOURCE      1	/* tell headers to include POSIX stuff */
#define _MINIX             1	/* tell headers to include MINIX stuff */
#define _SYSTEM            1    /* get OK and negative error codes */

#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/ipc.h>
#include <minix/endpoint.h>
#include <minix/sysutil.h>
#include <minix/config.h>
#include <minix/const.h>
#include <minix/type.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/safecopies.h>
#include <minix/chardriver.h>
#include <minix/ds.h>
#include <minix/list.h>
#include <minix/rs.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/rbtree.h>
#include <sys/mman.h>
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

static cp_grant_id_t grant_binder(endpoint_t end, int access, vir_bytes addr, size_t len);
static int get_binder_endpt(endpoint_t *pt);
int do_dum_server_call_back(message *);

#endif /* _DUM_SERVER_H */
