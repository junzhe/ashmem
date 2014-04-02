#define _SYSTEM	1
#define _MINIX 1
#include <sys/cdefs.h>
#include <lib.h>
#include "namespace.h"

#include <minix/rs.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ashmem.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

static int get_ashmem_endpt(endpoint_t *pt)
{
	return minix_rs_lookup("ashmem", pt);
}

int ashmem_create_region(const char *name, size_t size)
{
	printf("Call ashmem_create_region\n");
	message m1, m2;
	endpoint_t ashmem_pt;
	int r, id;

	if(get_ashmem_endpt(&ashmem_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	r = _syscall(ashmem_pt, ASHMEM_CREATE, &m1);

	if(r != OK)
		return r;

	id = m1.ASHMEM_CREATE_RETID;
	
	m2.ASHMEM_SET_SIZE_ID = id;
	m2.ASHMEM_SET_SIZE_SIZE = size;

	return _syscall(ashmem_pt, ASHMEM_SET_SIZE, &m2);
}

void *ashmem_mmap(void *addr, size_t length, int prot,
                int flags, int id, size_t offset)
{
	printf("Call ashmem_mmap\n");

	message m;
	endpoint_t ashmem_pt;
	int r;

	if(get_ashmem_endpt(&ashmem_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.ASHMEM_MMAP_ID = id;
	m.ASHMEM_MMAP_ADDR = (long) addr;
	m.ASHMEM_MMAP_FLAG = flags;

	r = _syscall(ashmem_pt, ASHMEM_MMAP, &m);

	if(r != OK)
		return r;

	return m.ASHMEM_MMAP_RETADDR;
}

int ashmem_set_prot_region(int id, int prot)
{
	printf("Call ashmem_set_prot_region\n");

	message m;
	endpoint_t ashmem_pt;

	if(get_ashmem_endpt(&ashmem_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.ASHMEM_SET_PROT_ID = id;
	m.ASHMEM_SET_PROT_PROT = prot;
	
	return _syscall(ashmem_pt, ASHMEM_SET_PROT, &m);
}

int ashmem_pin_region(int id, size_t offset, size_t len)
{
	printf("Call ashmem_pin_region\n");

	message m;
	endpoint_t ashmem_pt;

	if(get_ashmem_endpt(&ashmem_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.ASHMEM_PIN_ID = id;
	m.ASHMEM_PIN_OFFSET = offset;
	m.ASHMEM_PIN_LEN = len;

	return _syscall(ashmem_pt, ASHMEM_PIN, &m);
}

int ashmem_unpin_region(int id, size_t offset, size_t len)
{
	printf("Call ashmem_unpin_region\n");

	message m;
	endpoint_t ashmem_pt;

	if(get_ashmem_endpt(&ashmem_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.ASHMEM_UNPIN_ID = id;
	m.ASHMEM_UNPIN_OFFSET = offset;
	m.ASHMEM_UNPIN_LEN = len;

	return _syscall(ashmem_pt, ASHMEM_PIN, &m);
}

int ashmem_get_size_region(int id)
{
	printf("Call ashmem_get_size_region\n");

	message m;
	endpoint_t ashmem_pt;

	if(get_ashmem_endpt(&ashmem_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}
	
	m.ASHMEM_GET_SIZE_ID = id;

	return _syscall(ashmem_pt, ASHMEM_GET_SIZE, &m);
}
