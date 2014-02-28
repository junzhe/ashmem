/*	sys/ioc_block.h - Block ioctl() command codes.
 *
 */

#ifndef _S_I_ASHMEM_H
#define _S_I_ASHMEM_H

#include <minix/ioctl.h>

#define	ASHMEM_IOC_SET_NAME 	_IOW('a', 1, char *)
#define ASHMEM_IOC_SET_SIZE	_IOW('a', 2, size_t)

#endif /* _S_I_ASHMEM_H */
