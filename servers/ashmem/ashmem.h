#define _POSIX_SOURCE      1	/* tell headers to include POSIX stuff */
#define _MINIX             1	/* tell headers to include MINIX stuff */
#define _SYSTEM            1    /* get OK and negative error codes */

#define ASHMEM_NAME_LEN 256
#define ASHMEM_MAJOR 30

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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
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

/*
struct ashmem_area
{
};

struct ashmem_range
{
};

struct shrink_control
{
};

int range_alloc(struct ashmem_area *asma,
			struct ashmem_range *prev_range, unsigned int purged,
							size_t start, size_t end);

void range_del(struct ashmem_range *range);

void range_shrink(struct ashmem_range *range,
                                size_t start, size_t end);

int ashmem_open(struct inode *inode, struct file *file);

int ashmem_release(struct inode *ignored, struct file *file);

ssize_t ashmem_read(struct file *file, char __user *buf,
                           size_t len, loff_t *pos);

loff_t ashmem_llseek(struct file *file, loff_t offset, int origin);

int ashmem_mmap(struct file *file, struct vm_area_struct *vma);

int ashmem_shrink(struct shrinker *s, struct shrink_control *sc);

int set_prot_mask(struct ashmem_area *asma, unsigned long prot);

int set_name(struct ashmem_area *asma, void __user *name);

int get_name(struct ashmem_area *asma, void __user *name);

int ashmem_pin(struct ashmem_area *asma, size_t pgstart, size_t pgend);

int ashmem_unpin(struct ashmem_area *asma, size_t pgstart, size_t pgend);

int ashmem_get_pin_status(struct ashmem_area *asma, size_t pgstart,
                                 size_t pgend);

int ashmem_pin_unpin(struct ashmem_area *asma, unsigned long cmd,
                            void __user *p);

int __init ashmem_init(void);

void __exit ashmem_exit(void);
*/
