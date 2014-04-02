#include "zygote.h"

endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

static struct {
	int type;
	int (*func)(message *);
	int reply;      /* whether the reply action is passed through */
	} zygote_calls[] = {
	{ ZYGOTE_UP,		do_zygote_up, 		0 },
	{ ZYGOTE_DOWN,  	do_zygote_down, 	0 },
	{ ZYGOTE_REFRESH, 	do_zygote_refresh, 	0 },
	{ ZYGOTE_SHUTDOWN, 	do_zygote_shutdown, 	0 },
	{ ZYGOTE_UPDATE, 	do_zygote_update, 	0 },
	{ ZYGOTE_CLONE,		do_zygote_clone,	0 },
	{ ZYGOTE_EDIT,		do_zygote_edit,		0 },
	{ ZYGOTE_LOOKUP,	do_zygote_lookup,	0 }

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

int main(int argc, char *argv[])
{
	message m;

	/* SEF local startup. */
	env_setargs(argc, argv);
	sef_local_startup();

	while(TRUE) {
		int r;
		int zygote_number;

		if ((r = sef_receive(ANY, &m)) != OK)
			printf("sef_receive failed %d.\n", r);
		who_e = m.m_source;
		call_type = m.m_type;

		if(verbose)
			printf("ZYGOTE: get %d from %d\n", call_type, who_e);

	     	zygote_number = call_type - (ZYGOTE_BASE + 1);
		printf("ZYGOTE zygote_number: %d\n", zygote_number);
	
		/* dispatch message */
		if (zygote_number >= 0 && zygote_number < SIZE(zygote_calls)) {
			int result;

			if (zygote_calls[zygote_number].type != call_type)
				panic("ZYGOTE: call table order mismatch");

			/* If any process does an IPC call,
			 * we have to know about it exiting.
			 * Tell VM to watch it for us.
			 */
		  	if(vm_watch_exit(m.m_source) != OK) {
				printf("ZYGOTE: watch failed on %d\n", m.m_source);
		   	}

		   	result = zygote_calls[zygote_number].func(&m);

		   	/*
			 * The handler of the IPC call did not
			 * post a reply.
			 */
		      	if (!zygote_calls[zygote_number].reply) {

				m.m_type = result;

			        if(verbose && result != OK)
			      		printf("ZYGOTE: error for %d: %d\n",
							call_type, result);

				if ((r = sendnb(who_e, &m)) != OK)
					printf("ZYGOTE send error %d.\n", r);
		        }
	        } else {
		       /* warn and then ignore */
		       printf("ZYGOTE unknown call type: %d from %d.\n",
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
		printf("ZYGOTE: self: %d\n", SELF_E);

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

int do_zygote_up(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_up.\n");

	return _syscall(RS_PROC_NR, RS_UP, m);
}

int do_zygote_down(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_down.\n");

	return _syscall(RS_PROC_NR, RS_DOWN, m);
}

int do_zygote_refresh(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_refresh.\n");

	return _syscall(RS_PROC_NR, RS_REFRESH, m);
}

int do_zygote_restart(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_restart.\n");

	return _syscall(RS_PROC_NR, RS_RESTART, m);
}

int do_zygote_shutdown(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_shutdown.\n");

	return _syscall(RS_PROC_NR, RS_SHUTDOWN, m);
}

int do_zygote_update(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_update.\n");

	return _syscall(RS_PROC_NR, RS_UPDATE, m);
}

int do_zygote_clone(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_clone.\n");

	return _syscall(RS_PROC_NR, RS_CLONE, m);
}

int do_zygote_edit(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_edit.\n");

	return _syscall(RS_PROC_NR, RS_EDIT, m);
}

int do_zygote_lookup(message *m)
{
	if(verbose)
		printf("ZYGOTE do_zygote_lookup.\n");

	return _syscall(RS_PROC_NR, RS_LOOKUP, m);
}











