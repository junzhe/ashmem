#define _MINIX 1
#define _SYSTEM 1

#include <sys/cdefs.h>
#include <lib.h>
#include <unistd.h>

#include <minix/com.h>
#include <minix/rs.h>
#include <minix/binder.h>
#include <minix/safecopies.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static int get_binder_endpt(endpoint_t *pt)
{
	return minix_rs_lookup("binder", pt);
}

vir_bytes create_binder(size_t size)
{
	message m;
	endpoint_t binder_pt;

	int r;

	if (get_binder_endpt(&binder_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.BINDER_CREATE_SIZE = size;
	m.BINDER_CREATE_PID = getpid();

	r = _syscall(binder_pt, BINDER_CREATE, &m);
	if (r != OK)
		return r;

	return (vir_bytes) m.BINDER_CREATE_RETADDR;
}

int binder_read_kernel(struct binder_write_read *data)
{
	message m;
	endpoint_t binder_pt;
	int r;

	if(get_binder_endpt(&binder_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.BINDER_READ_KERNEL_CODE = data->code;
	m.BINDER_READ_KERNEL_BINDER_REF =  data->binder_ref;
	m.BINDER_READ_KERNEL_BINDER_ADDR = (long) data->binder_addr;

	r = _syscall(binder_pt, BINDER_READ_KERNEL, &m);

	data->data = m.BINDER_READ_KERNEL_DATA_BUFF;
	data->offset = m.BINDER_READ_KERNEL_OFFSET_BUFF;
	data->data_size = m.BINDER_READ_KERNEL_DATA_SIZE;
	data->offset_size = m.BINDER_READ_KERNEL_OFFSET_SIZE;

	return r;
}

int binder_read(struct binder_write_read *data)
{
	message m;
	endpoint_t binder_pt;
	int r;

	if(get_binder_endpt(&binder_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.BINDER_READ_CODE = data->code;
	m.BINDER_READ_BINDER_REF =  data->binder_ref;
	m.BINDER_READ_BINDER_ADDR = (long) data->binder_addr;

	r = _syscall(binder_pt, BINDER_READ, &m);

	data->data = m.BINDER_READ_DATA_BUFF;
	data->offset = m.BINDER_READ_OFFSET_BUFF;
	data->data_size = m.BINDER_READ_DATA_SIZE;
	data->offset_size = m.BINDER_READ_OFFSET_SIZE;

	return r;
}

int binder_write_kernel(struct binder_write_read *data)
{
	message m;
        endpoint_t binder_pt;

	int r;

	if (get_binder_endpt(&binder_pt) != OK) {  
		errno = ENOSYS;
		return -1;
	}

	m.BINDER_WRITE_KERNEL_DATA_SIZE = data->data_size;
	m.BINDER_WRITE_KERNEL_DATA_BUFF = (long) data->data;
	m.BINDER_WRITE_KERNEL_OFFSET_SIZE = data->offset_size;
	m.BINDER_WRITE_KERNEL_OFFSET_BUFF = (long) data->offset;
	m.BINDER_WRITE_KERNEL_CODE = data->code;
	m.BINDER_WRITE_KERNEL_BINDER_REF =  data->binder_ref;
	m.BINDER_WRITE_KERNEL_BINDER_ADDR = (long) data->binder_addr;

	r = _syscall(binder_pt, BINDER_WRITE_KERNEL, &m);

	return r;
}

int binder_write(struct binder_write_read *data)
{
	message m;
	endpoint_t binder_pt;

	int r;

	if (get_binder_endpt(&binder_pt) != OK) {
		errno = ENOSYS;
		return -1;
	}

	m.BINDER_WRITE_DATA_SIZE = data->data_size;
	m.BINDER_WRITE_DATA_GRANT = (long) data->data_g_key;
	m.BINDER_WRITE_OFFSET_SIZE = data->offset_size;
	m.BINDER_WRITE_OFFSET_GRANT = (long) data->offset_g_key;
	m.BINDER_WRITE_CODE = data->code;
	m.BINDER_WRITE_BINDER_REF =  data->binder_ref;
	m.BINDER_WRITE_BINDER_ADDR = (long) data->binder_addr;

	r = _syscall(binder_pt, BINDER_WRITE, &m);

	return r;
}
