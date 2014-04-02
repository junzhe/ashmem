#ifndef BINDER_H
#define BINDER_H

#define ASERMAN_REF	0
#define MAP_SIZE 4096
#include <minix/type.h>
#include <minix/com.h>
#include <sys/types.h>
#include <minix/safecopies.h>

struct binder_write_read {
	long binder_ref;
	void *binder_addr;

	unsigned int code;

	size_t read_size;
	size_t write_size;

	size_t data_size;
	size_t offset_size;

	cp_grant_id_t data_g_key;
	cp_grant_id_t offset_g_key;

	void *data;
	void *offset;
};

vir_bytes create_binder(size_t size);
int binder_read(struct binder_write_read *data);
int binder_write(struct binder_write_read *data);

int binder_read_kernel(struct binder_write_read *data);
int binder_write_kernel(struct binder_write_read *data);

#endif
