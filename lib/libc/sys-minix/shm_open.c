#define _SYSTEM	1
#define _MINIX 1
#include <sys/cdefs.h>
#include <lib.h>
#include "namespace.h"

#include <minix/rs.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>


#ifdef __weak_alias
__weak_alias(shm_open, _shm_open)
#endif


static int get_ipc_endpt(endpoint_t *pt)
{
	return minix_rs_lookup("ashmem", pt);
}

/* Get shared memory segment. */
int shm_open(const char* name, int flag, mode_t mode)
{
	message m;
	endpoint_t ipc_pt;
	int r;

	if (get_ipc_endpt(&ipc_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	printf("%s\n", "lib_shm_open ");
	
	r = _syscall(ipc_pt, ASHMEM_CREATE, &m);

	return r;
}
