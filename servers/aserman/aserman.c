#include "aserman.h"

endpoint_t who_e;
int call_type;
endpoint_t SELF_E;
endpoint_t binder_pt;
vir_bytes binder_node;

static struct {
	int type;
	int (*func)(message *);
	int reply;      /* whether the reply action is passed through */
	} aserman_calls[] = {
	{ ASERMAN_BINDER_READ,	do_binder_read, 	0 },
	{ ASERMAN_BINDER_WRITE,	do_binder_write,	0 },
	{ ASERMAN_BINDER_READ_KERNEL, do_binder_read_kernel, 0},
	{ ASERMAN_BINDER_WRITE_KERNEL, do_binder_write_kernel, 0}
};

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

static int verbose = 0;

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void sef_cb_signal_handler(int signo);

int minix_rs_lookup(const char *name, endpoint_t *value)
{
        message m;
	size_t len_key;

	len_key = strlen(name)+1;

	m.RS_NAME = (char *) __UNCONST(name);
	m.RS_NAME_LEN = len_key;

	if (_syscall(RS_PROC_NR, RS_LOOKUP, &m) != -1) {
		*value = m.RS_ENDPOINT;
		return OK;
	}

	return -1;
}

static int get_binder_endpt(endpoint_t *pt)
{
        return minix_rs_lookup("binder", pt);
}

int main(int argc, char *argv[])
{
	message m;
	
	/* SEF local startup. */
	env_setargs(argc, argv);
	sef_local_startup();

	if (get_binder_endpt(&binder_pt) != OK) {
		printf("ASERMAN get binder fail\n");
	}

	binder_node = create_binder(PAGE_SIZE - 1);

	while(TRUE) {
		if(verbose)
			printf("aserman start.\n"); 
		
		int r;
		int aserman_number;

		if ((r = sef_receive(ANY, &m)) != OK)
			printf("sef_receive failed %d.\n", r);
		who_e = m.m_source;
		call_type = m.m_type;

		if(verbose)
			printf("aserman: get %d from %d\n", call_type, who_e);

	     	aserman_number = call_type - (ASERMAN_BASE + 1);
		
		if(verbose)
			printf("ASERMAN aserman_number: %d\n", aserman_number);
	
		/* dispatch message */
		if (aserman_number >= 0 && aserman_number < SIZE(aserman_calls)) {
			int result;

			if (aserman_calls[aserman_number].type != call_type)
				panic("ASERMAN: call table order mismatch");

			/* If any process does an IPC call,
			 * we have to know about it exiting.
			 * Tell VM to watch it for us.
			 */
		  	if(vm_watch_exit(m.m_source) != OK) {
				printf("ASERMAN: watch failed on %d\n", m.m_source);
		   	}

		   	result = aserman_calls[aserman_number].func(&m);

		   	/*
			 * The handler of the IPC call did not
			 * post a reply.
			 */
		      	if (!aserman_calls[aserman_number].reply) {

				m.m_type = result;

			        if(verbose && result != OK)
			      		printf("ASERMAN: error for %d: %d\n",
							call_type, result);

				if ((r = sendnb(who_e, &m)) != OK)
					printf("ASERMAN send error %d.\n", r);
		        }
	        } else {
		       /* warn and then ignore */
		       printf("ASERMAN unknown call type: %d from %d.\n",
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
		printf("ASERMAN: self: %d\n", SELF_E);

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

int do_binder_read(message *m)
{
	if(verbose)
		printf("ASERMAN call do_binder_read.\n");

	int * data_addr = (int *) m->ASERMAN_BINDER_READ_DATA_ADDR;

	if(verbose)
		printf("ASERMAN binder_read %d\n", *data_addr);

	return OK;
}

int do_binder_read_kernel(message *m)
{
	if(verbose)
		printf("ASERMAN do_binder_read_kernel.\n");

	int * data_addr = (int *) m->ASERMAN_BINDER_READ_KERNEL_DATA_ADDR;

	if(verbose)
		printf("ASERMAN binder_read_kernel %d\n", *data_addr);

	return OK;
}

int do_binder_write(message *m)
{
	if(verbose)
		printf("ASERMAN call do_binder_write.\n");
	
	int *data = malloc(4);
	int *offset = malloc(4);

	*data = 100;
	*offset = 0;

	cp_grant_id_t data_g_key, offset_g_key;

	data_g_key = cpf_grant_direct(who_e, (vir_bytes) data, (vir_bytes) 4, CPF_READ);
	offset_g_key = cpf_grant_direct(who_e, (vir_bytes) offset, (vir_bytes) 4, CPF_READ); 

	m->ASERMAN_BINDER_WRITE_DATA_GRANT = data_g_key;
	m->ASERMAN_BINDER_WRITE_OFFSET_GRANT = offset_g_key;
	m->ASERMAN_BINDER_WRITE_DATA_SIZE = sizeof(data);
	m->ASERMAN_BINDER_WRITE_OFFSET_SIZE = sizeof(offset);

	return OK;
}
int do_binder_write_kernel(message *m)
{
	if(verbose)
		printf("ASERMAN do_binder_write_kernel.\n");

	int *data = malloc(4);
	int *offset = malloc(4);

	*data = 100;
	*offset = 0;

	m->ASERMAN_BINDER_WRITE_KERNEL_DATA_ADDR = data;
	m->ASERMAN_BINDER_WRITE_KERNEL_OFFSET_ADDR = offset;
	m->ASERMAN_BINDER_WRITE_KERNEL_DATA_SIZE = 4;
	m->ASERMAN_BINDER_WRITE_KERNEL_OFFSET_SIZE = 4;
	return OK;
}
