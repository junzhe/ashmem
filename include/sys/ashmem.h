#ifndef __SYS_ASHMEM_H__
#define __SYS_ASHMEM_H__

/*
sys/ashmem.h
*/
#include <sys/types.h>

int ashmem_create_region(const char *name, size_t size);
int ashmem_set_prot_region(int id, int prot);
int ashmem_pin_region(int id, size_t offset, size_t len);
int ashmem_unpin_region(int id, size_t offset, size_t len);
int ashmem_get_size_region(int id);
void *ashmem_mmap(void *addr, size_t length, int prot, 
		int flags, int id, size_t offset);

#endif /* __SYS_ASHMEM_H__ */
