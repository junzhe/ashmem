#define BINDER_MESSAGE "Binder called\n"
#define MAX_BINDER_N 1024
#define BINDER_BUFFER_SIZE 4096
#define SIZE(a) (sizeof(a)/sizeof(a[0]))

#include "binder.h"

endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

static int verbose = 0;

static struct {
        int type;
	int (*func)(message *);
	int reply;      /* whether the reply action is passed through */
	} binder_calls[] = {
	{ BINDER_CREATE,	do_binder_create,		0 },
	{ BINDER_READ,		do_binder_read,			0 },
	{ BINDER_WRITE,		do_binder_write,		0 },
	{ BINDER_READ_KERNEL,	do_binder_read_kernel,		0 },
	{ BINDER_WRITE_KERNEL,	do_binder_write_kernel,		0 }
};

struct binder_proc {
 	long hard_ref; 
	long id;
	pid_t pid;
	size_t size;
	vir_bytes baddr;
	vir_bytes addr;
	phys_bytes paddr;
	endpoint_t who_e;
};

static HLIST_HEAD(binder_procs);
static struct binder_proc binder_proc_list[MAX_BINDER_N];
static int binder_proc_list_n = 0;

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void sef_cb_signal_handler(int signo);

int main(int argc, char *argv[])
{
        message m;

        /* SEF local startup. */
        env_setargs(argc, argv);
        sef_local_startup();

        while(TRUE) {
                if(verbose)
			printf("%s",BINDER_MESSAGE);
                
		int r;
                int binder_number;

                if ((r = sef_receive(ANY, &m)) != OK)
                        printf("sef_receive failed %d.\n", r);
                who_e = m.m_source;
                call_type = m.m_type;

                if(verbose)
                        printf("BINDER: get %d from %d\n", call_type, who_e);

                binder_number = call_type - (BINDER_BASE + 1);
                
		if(verbose)
			printf("BINDER binder_number: %d\n", binder_number);

                /* dispatch message */
                if (binder_number >= 0 && binder_number < SIZE(binder_calls)) {
                        int result;

                        if (binder_calls[binder_number].type != call_type)
                                panic("BINDER: call table order mismatch");

                        /* If any process does an IPC call,
                         * we have to know about it exiting.
                         * Tell VM to watch it for us.
                         */
                        if(vm_watch_exit(m.m_source) != OK) {
                                printf("BINDER: watch failed on %d\n", m.m_source);
                        }

                        result = binder_calls[binder_number].func(&m);

                        /*
                         * The handler of the IPC call did not
                         * post a reply.
                         */
                        if (!binder_calls[binder_number].reply) {

                                m.m_type = result;

                                if(verbose && result != OK)
                                        printf("BINDER: error for %d: %d\n",
                                                        call_type, result);

                                if ((r = sendnb(who_e, &m)) != OK)
                                        printf("BINDER send error %d.\n", r);
                        }
                } else {
                       /* warn and then ignore */
                       printf("BINDER unknown call type: %d from %d.\n",
                                                      call_type, who_e);
                }
	}
        return -1;
}

/*===========================================================================*
 *                             sef_local_startup                             *
 *===========================================================================*/
static void sef_local_startup()
{
        /* Register init callbacks. */
        sef_setcb_init_fresh(sef_cb_init_fresh);
        sef_setcb_init_restart(sef_cb_init_fresh);

        /* No live update support for now. */

        /* Register signal callbacks. */
        sef_setcb_signal_handler(sef_cb_signal_handler);

        /* Let SEF perform startup. */
        sef_startup();
}

/*===========================================================================*
 *                          sef_cb_init_fresh                                *
 *===========================================================================*/
static int sef_cb_init_fresh(int UNUSED(type), sef_init_info_t *UNUSED(info))
{
        /* Initialize the ipc server. */

        SELF_E = getprocnr();

        if(verbose)
                printf("BINDER: self: %d\n", SELF_E);

        return(OK);
}

/*===========================================================================*
 *                          sef_cb_signal_handler                            *
 *===========================================================================*/
static void sef_cb_signal_handler(int signo)
{
        /* Only check for termination signal, ignore anything else. */
        if (signo != SIGTERM) return;
}

struct binder_proc *get_binder_proc()
{
	struct binder_proc *binder = &binder_proc_list[binder_proc_list_n++];
	
	memset(binder, 0, sizeof(struct binder_proc));

	binder->id = binder_proc_list_n - 1;

	return binder;
}

struct binder_proc *get_binder_by_id(int id)
{
	return &binder_proc_list[id];
}

struct binder_proc *get_binder_by_hard_ref(long hard_ref, endpoint_t who_e)
{
	int i;
	for(i = 0; i < binder_proc_list_n; i++) {
		if(binder_proc_list[i].hard_ref == hard_ref && binder_proc_list[i].who_e == who_e)
			return &binder_proc_list[i];
	}

	return NULL;
}

int do_binder_create(message *m)
{
	size_t size;
	pid_t	pid;

	size = m->BINDER_CREATE_SIZE;
	
	pid = m->BINDER_CREATE_PID;

	struct binder_proc *binder = get_binder_proc();

	binder->size = size;

	if (size % PAGE_SIZE)
		size += PAGE_SIZE - size % PAGE_SIZE;

	binder->baddr = (vir_bytes) minix_mmap(0, size,
				PROT_READ|PROT_WRITE, MAP_ANON, -1, 0);

	binder->paddr = vm_getphys(SELF_E, (void *) binder->baddr);

	memset((void *) binder->baddr, 0, size);

	binder->hard_ref = binder->addr = (long) vm_remap(who_e, SELF_E, NULL, (void *) binder->baddr,
                                binder->size);

	if(binder->addr == (long) MAP_FAILED) {
		printf("BINDER vm_remap failed.\n");
		return ENOMEM;
	}

	binder->who_e = who_e;
	
	binder->pid = pid;

	m->BINDER_CREATE_RETID = binder->id;
	
	m->BINDER_CREATE_RETADDR = (char *) binder->addr;

	return OK;
}

int do_binder_read(message *m)
{
	long id;
	long hard_ref;
	int code;
	struct binder_proc *binder, *binder_src;
	size_t offset_size, data_size;
	vir_bytes src_data_ptr, src_offset_ptr, dst_data_ptr, dst_offset_ptr, data_ptr, offset_ptr;
	cp_grant_id_t data_g_key, offset_g_key;

	int r;
	
	code = m->BINDER_WRITE_CODE;

	id = (long) m->BINDER_READ_BINDER_REF;
	binder = get_binder_by_id(id);

	hard_ref = (long) m->BINDER_READ_BINDER_ADDR;
	binder_src = get_binder_by_hard_ref(hard_ref, who_e);

	message m1;

	m1.ASERMAN_BINDER_WRITE_CODE = code;

	r = _syscall(binder->who_e, ASERMAN_BINDER_WRITE, &m1);

	if(r != OK) {
		printf("BINDER call_back error.");
		return ENOMEM;
	} 

	data_g_key = m1.ASERMAN_BINDER_WRITE_DATA_GRANT;
	offset_g_key = m1.ASERMAN_BINDER_WRITE_OFFSET_GRANT;

	data_size = m1.ASERMAN_BINDER_WRITE_DATA_SIZE;
	offset_size = m1.ASERMAN_BINDER_WRITE_OFFSET_SIZE;

	src_data_ptr = binder_src->baddr;
	src_offset_ptr = src_data_ptr + data_size;

	data_ptr = binder_src->addr;
	offset_ptr = data_ptr + data_size;

	r = sys_safecopyfrom(binder->who_e, data_g_key, (vir_bytes) 0, (vir_bytes) src_data_ptr, data_size);

	if(r != OK)
		printf("BINDER READ first sys_safecopyfrom failed.\n");

	r = sys_safecopyfrom(binder->who_e, offset_g_key, (vir_bytes) 0, (vir_bytes) src_offset_ptr, offset_size);

	if(r != OK)
		printf("BINDER READ second sys_safecopyfrom failed.\n");

	m->BINDER_READ_DATA_BUFF = data_ptr;
	m->BINDER_READ_OFFSET_BUFF = offset_ptr;
	m->BINDER_READ_DATA_SIZE = data_size;
	m->BINDER_READ_OFFSET_SIZE = offset_size;

	return OK;
}

int do_binder_write(message *m)
{
	long id;
	long hard_ref;
	int code;
	struct binder_proc *binder, *binder_src;
	size_t offset_size, data_size; 
	vir_bytes src_data_ptr, src_offset_ptr, dst_data_ptr, dst_offset_ptr, data_ptr, offset_ptr;
	cp_grant_id_t data_g_key, offset_g_key;
	int r;

	code = m->BINDER_WRITE_CODE;

	id = (long) m->BINDER_WRITE_BINDER_REF;
	binder = get_binder_by_id(id);
	
	hard_ref = (long) m->BINDER_WRITE_BINDER_ADDR;
	binder_src = get_binder_by_hard_ref(hard_ref, who_e);

	offset_size = m->BINDER_WRITE_DATA_SIZE;
	data_size = m->BINDER_WRITE_DATA_SIZE;

	data_g_key = m->BINDER_WRITE_DATA_GRANT;
	offset_g_key = m->BINDER_WRITE_OFFSET_GRANT;

	dst_data_ptr = binder->baddr;
	dst_offset_ptr = binder->baddr + data_size;

	r = sys_safecopyfrom(who_e, data_g_key, (vir_bytes) 0, (vir_bytes) dst_data_ptr, data_size);
	
	if(r != OK)
		printf("BINDER first sys_safecopyfrom failed.\n");

	r = sys_safecopyfrom(who_e, offset_g_key, (vir_bytes) 0, (vir_bytes) dst_offset_ptr, offset_size);

	if(r != OK)
		printf("BINDER second sys_safecopyfrom failed.\n");

	data_ptr = binder->addr;
	offset_ptr = data_ptr + data_size;

	/*
	src_data_ptr = binder_src->baddr;
	src_offset_ptr = binder_src->baddr + data_size;

	r = memcpy(dst_data_ptr, src_data_ptr, data_size);

	if(r == -1)
		printf("BINDER memcpy failed.\n");

	r = memcpy(dst_offset_ptr, src_offset_ptr, offset_size);

	if(r == -1)
		printf("BINDER memcpy failed.\n");
	*/

	message m1;

	m1.ASERMAN_BINDER_READ_DATA_ADDR = (char *) data_ptr;
	m1.ASERMAN_BINDER_READ_DATA_SIZE = data_size;
	m1.ASERMAN_BINDER_READ_OFFSET_ADDR = (char *) offset_ptr;
	m1.ASERMAN_BINDER_READ_OFFSET_SIZE = offset_size;
	m1.ASERMAN_BINDER_READ_CODE = code;

	r = _syscall(binder->who_e, ASERMAN_BINDER_READ, &m1);

	if(r != OK) {
		printf("BINDER call_back error.");
		return ENOMEM;
	}

	return OK;

}

int do_binder_read_kernel(message *m)
{
	if(verbose)
		printf("BINDER binder_read_kernel.\n");

	long id;
	long hard_ref;
	int code;
	struct binder_proc *binder, *binder_src;
	size_t offset_size, data_size;
	vir_bytes src_data_ptr, src_offset_ptr, dst_data_ptr, dst_offset_ptr, data_ptr, offset_ptr;
	cp_grant_id_t data_g_key, offset_g_key;

	int r;

	code = m->BINDER_WRITE_KERNEL_CODE;

	id = (long) m->BINDER_READ_KERNEL_BINDER_REF;
	binder = get_binder_by_id(id);

	hard_ref = (long) m->BINDER_READ_KERNEL_BINDER_ADDR;
	binder_src = get_binder_by_hard_ref(hard_ref, who_e);

	message m1;

	m1.ASERMAN_BINDER_WRITE_KERNEL_CODE = code;

	r = _syscall(binder->who_e, ASERMAN_BINDER_WRITE_KERNEL, &m1);

	if(r != OK) {
		printf("BINDER call_back error.");
		return ENOMEM;
	}

	src_data_ptr = m1.ASERMAN_BINDER_WRITE_KERNEL_DATA_ADDR;
	src_offset_ptr = m1.ASERMAN_BINDER_WRITE_KERNEL_OFFSET_ADDR;

	data_size = m1.ASERMAN_BINDER_WRITE_KERNEL_DATA_SIZE;
	offset_size = m1.ASERMAN_BINDER_WRITE_KERNEL_OFFSET_SIZE;

	data_ptr = binder_src->addr;
	offset_ptr = data_ptr + data_size;

	sys_bindercopy(who_e, binder->who_e, data_ptr, src_data_ptr, data_size, 0, 0);
	sys_bindercopy(who_e, binder->who_e, offset_ptr, src_offset_ptr, offset_size, 0, 0);

	m->BINDER_READ_KERNEL_DATA_BUFF = data_ptr;
	m->BINDER_READ_KERNEL_OFFSET_BUFF = offset_ptr;
	m->BINDER_READ_KERNEL_DATA_SIZE = data_size;
	m->BINDER_READ_KERNEL_OFFSET_SIZE = offset_size;

	return OK;
}

int do_binder_write_kernel(message *m)
{
	if(verbose)
		printf("BINDER binder_write_kernel.\n");
	
	long id;
	int code, r;
	size_t offset_size, data_size;
	struct binder_proc *binder;
	vir_bytes src_data_ptr, dst_data_ptr, src_offset_ptr, dst_offset_ptr;

	code = m->BINDER_WRITE_KERNEL_CODE;

	id = (long) m->BINDER_WRITE_KERNEL_BINDER_REF;
	binder = get_binder_by_id(id);

	offset_size = m->BINDER_WRITE_KERNEL_OFFSET_SIZE;
	data_size = m->BINDER_WRITE_KERNEL_DATA_SIZE;

	src_data_ptr = m->BINDER_WRITE_KERNEL_DATA_BUFF;
	src_offset_ptr = m->BINDER_WRITE_KERNEL_OFFSET_BUFF;

	dst_data_ptr = binder->addr;
	dst_offset_ptr = binder->addr + data_size;

	r = sys_bindercopy(binder->who_e, who_e, dst_data_ptr, src_data_ptr, data_size, 0, 0);
	r = sys_bindercopy(binder->who_e, who_e, dst_offset_ptr, src_offset_ptr, offset_size, 0, 0);

	message m1;

	m1.ASERMAN_BINDER_READ_KERNEL_DATA_ADDR = (char *) dst_data_ptr;
	m1.ASERMAN_BINDER_READ_KERNEL_DATA_SIZE = data_size;
	m1.ASERMAN_BINDER_READ_KERNEL_OFFSET_ADDR = (char *) dst_offset_ptr;
	m1.ASERMAN_BINDER_READ_KERNEL_OFFSET_SIZE = offset_size;
	m1.ASERMAN_BINDER_READ_KERNEL_CODE = code;

	r = _syscall(binder->who_e, ASERMAN_BINDER_READ_KERNEL, &m1);

	if(r != OK) {
		printf("BINDER call_back error.");
		return ENOMEM;
	}

	return OK;
}
