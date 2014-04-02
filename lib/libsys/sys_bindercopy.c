#include "syslib.h"

int sys_bindercopy(endpoint_t dst_t, endpoint_t src_t, vir_bytes dst_addr, vir_bytes src_addr, size_t size, size_t dst_offset, size_t
src_offset)
{
        message m;

	m.BINDERCOPY_DST_END = dst_t;
	m.BINDERCOPY_SRC_END = src_t;
	m.BINDERCOPY_DST_ADDR = dst_addr;
	m.BINDERCOPY_SRC_ADDR = src_addr;
	m.BINDERCOPY_SIZE = size;

	return(_kernel_call(SYS_BINDERCOPY, &m));
}
