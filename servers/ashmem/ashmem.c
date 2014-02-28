#define ASHMEM_MESSAGE "Ashmem called\n"
#define MAX_ASHMEM_N 1024
#define ASHMEM_NAME_PREFIX "dev/ashmem/"
#define ASHMEM_NAME_PREFIX_LEN (sizeof(ASHMEM_NAME_PREFIX) - 1)
#define ASHMEM_FULL_NAME_LEN (ASHMEM_NAME_LEN + ASHMEM_NAME_PREFIX_LEN)

#include "ashmem.h"

endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

static struct {
	int type;
	int (*func)(message *);
	int reply;      /* whether the reply action is passed through */
	} ashmem_calls[] = {
	{ ASHMEM_CREATE,	do_ashmem_create_region,	0 },
	{ ASHMEM_RELEASE,	do_ashmem_release_region,	0 },
	{ ASHMEM_SET_NAME,	do_ashmem_set_name_region,	0 },
	{ ASHMEM_SET_SIZE,	do_ashmem_set_size_region,	0 },
	{ ASHMEM_MMAP,		do_ashmem_mmap_region, 		0 },
};

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

static int verbose = 1;

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void sef_cb_signal_handler(int signo);

struct ashmem_struct {
	char name[ASHMEM_FULL_NAME_LEN];
	int id;
	struct shmid_ds shmid_ds;
	vir_bytes page;
	int vm_id;
}; 

static struct ashmem_struct ashmem_list[MAX_ASHMEM_N];
static int ashmem_list_n = 0;

int main(int argc, char *argv[])
{
	message m;

	/* SEF local startup. */
	env_setargs(argc, argv);
	sef_local_startup();

	while(TRUE) {
		printf("%s",ASHMEM_MESSAGE); 
		int r;
		int ashmem_number;

		if ((r = sef_receive(ANY, &m)) != OK)
			printf("sef_receive failed %d.\n", r);
		who_e = m.m_source;
		call_type = m.m_type;

		if(verbose)
			printf("ASHMEM: get %d from %d\n", call_type, who_e);

		/*
		 * The ipc number in the table can be obtained
	 	 * with a simple equation because the values of
	         * IPC system calls are consecutive and begin
	         * at ( ASHMEM_BASE + 1 )
	         */

	     	ashmem_number = call_type - (ASHMEM_BASE + 1);
		printf("ASHMEM ashmem_number: %d\n", ashmem_number);
	
		/* dispatch message */
		if (ashmem_number >= 0 && ashmem_number < SIZE(ashmem_calls)) {
			int result;

			if (ashmem_calls[ashmem_number].type != call_type)
				panic("ASHMEM: call table order mismatch");

			/* If any process does an IPC call,
			 * we have to know about it exiting.
			 * Tell VM to watch it for us.
			 */
		  	if(vm_watch_exit(m.m_source) != OK) {
				printf("ASHMEM: watch failed on %d\n", m.m_source);
		   	}

		   	result = ashmem_calls[ashmem_number].func(&m);

		   	/*
			 * The handler of the IPC call did not
			 * post a reply.
			 */
		      	if (!ashmem_calls[ashmem_number].reply) {

				m.m_type = result;

			        if(verbose && result != OK)
			      		printf("ASHMEM: error for %d: %d\n",
							call_type, result);

				if ((r = sendnb(who_e, &m)) != OK)
					printf("ASHMEM send error %d.\n", r);
		        }
	        } else {
		       /* warn and then ignore */
		       printf("ASHMEM unknown call type: %d from %d.\n",
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
		printf("ASHMEM: self: %d\n", SELF_E);

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

static struct ashmem_struct *ashmem_find_area()
{
	struct ashmem_struct ashmem = ashmem_list[ashmem_list_n];
	
	ashmem.id = ashmem_list_n;

	ashmem_list_n++;

	return &ashmem;
}

int do_ashmem_create_region(message *m)
{
	printf("Call do_ashmem_create_region\n");

	struct ashmem_struct *ashmem;

	ashmem = ashmem_find_area();

	m->ASHMEM_CREATE_RETID = ashmem->id;

	return OK;
}

int do_ashmem_mmap_region(message *m)
{
	
}

int do_ashmem_release_region(message *m)
{
}

int do_ashmem_set_name_region(message *m)
{
}

int do_ashmem_set_size_region(message *m)
{
	
}

int do_ashmem_set_prot_region(message *m)
{
}

int do_ashmem_pin_region(message *m)
{
}

int do_ashmem_unpin_region(message *m)
{
}

int do_ashmem_get_size_region(message *m)
{
}

