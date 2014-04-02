#include "dum_server.h"

endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

static struct {
	int type;
	int (*func)(message *);
	int reply;      /* whether the reply action is passed through */
	} dum_server_calls[] = {
	{ DUM_SERVER_CALL_BACK,	do_dum_server_call_back, 	0 }
};

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

static int verbose = 1;

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

static cp_grant_id_t grant_binder(endpoint_t end, int access, vir_bytes addr, size_t len)
{
	cp_grant_id_t g_key;
	g_key = cpf_grant_direct(end, addr,
			len, access);

	if(!GRANT_VALID(g_key))
		printf("Grant error.\n");

	return g_key;
}

static int send_to_binder(endpoint_t binder_pt, cp_grant_id_t g_key, size_t len, vir_bytes offset)
{
	message m;
	m.BINDER_READ_GRANT_KEY = g_key;
	m.BINDER_READ_ADDR = 0;
	m.BINDER_READ_OFFSET = offset;
	m.BINDER_READ_SIZE = len;

	_syscall(binder_pt, BINDER_READ, &m);

	return 0;
}

int main(int argc, char *argv[])
{
	message m;
	endpoint_t binder_pt;
	cp_grant_id_t g_key;
	char msg[] = "hello binder!";

	/* SEF local startup. */
	env_setargs(argc, argv);
	sef_local_startup();

	if (get_binder_endpt(&binder_pt) != OK) {
		printf("DUM_SERVER get binder fail\n");
	}

	g_key = grant_binder(binder_pt,CPF_READ, (vir_bytes) msg, sizeof(msg));

	send_to_binder(binder_pt, g_key, 0, sizeof(msg));

	while(TRUE) {
		printf("dum_server start.\n"); 
		int r;
		int dum_server_number;

		if ((r = sef_receive(ANY, &m)) != OK)
			printf("sef_receive failed %d.\n", r);
		who_e = m.m_source;
		call_type = m.m_type;

		if(verbose)
			printf("dum_server: get %d from %d\n", call_type, who_e);

	     	dum_server_number = call_type - (DUM_SERVER_BASE + 1);
		printf("DUM_SERVER dum_server_number: %d\n", dum_server_number);
	
		/* dispatch message */
		if (dum_server_number >= 0 && dum_server_number < SIZE(dum_server_calls)) {
			int result;

			if (dum_server_calls[dum_server_number].type != call_type)
				panic("DUM_SERVER: call table order mismatch");

			/* If any process does an IPC call,
			 * we have to know about it exiting.
			 * Tell VM to watch it for us.
			 */
		  	if(vm_watch_exit(m.m_source) != OK) {
				printf("DUM_SERVER: watch failed on %d\n", m.m_source);
		   	}

		   	result = dum_server_calls[dum_server_number].func(&m);

		   	/*
			 * The handler of the IPC call did not
			 * post a reply.
			 */
		      	if (!dum_server_calls[dum_server_number].reply) {

				m.m_type = result;

			        if(verbose && result != OK)
			      		printf("DUM_SERVER: error for %d: %d\n",
							call_type, result);

				if ((r = sendnb(who_e, &m)) != OK)
					printf("DUM_SERVER send error %d.\n", r);
		        }
	        } else {
		       /* warn and then ignore */
		       printf("DUM_SERVER unknown call type: %d from %d.\n",
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
		printf("DUM_SERVER: self: %d\n", SELF_E);

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

int do_dum_server_call_back(message *m)
{
	printf("Call dum_server_call_back.\n");

	return OK;
}
