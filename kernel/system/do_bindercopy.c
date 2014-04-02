#include <assert.h>

#include "kernel/system.h"
#include "kernel/kernel.h"

#include <minix/endpoint.h>

/*===========================================================================*
 *                                do_bindercopy                              *
 *===========================================================================*/
int do_bindercopy(struct proc *caller_ptr, message *m_ptr)
{
	static struct vir_addr v_src, v_dst;

	v_src.proc_nr_e = m_ptr->BINDERCOPY_SRC_END;
	v_dst.proc_nr_e = m_ptr->BINDERCOPY_DST_END;

	v_src.offset = m_ptr->BINDERCOPY_SRC_ADDR;
	v_dst.offset = m_ptr->BINDERCOPY_DST_ADDR;
	
	return virtual_copy_vmcheck(caller_ptr, &v_src, &v_dst, m_ptr->BINDERCOPY_SIZE);
}
