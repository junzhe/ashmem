#include "dum_client.h"

endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

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

int main(int argc, char *argv[])
{
	message m;
	endpoint_t binder_pt;
	cp_grant_id_t data_g_key, offset_g_key;

	/* SEF local startup. */
	env_setargs(argc, argv);
	sef_local_startup();

	if (get_binder_endpt(&binder_pt) != OK) {
		printf("DUM_CLIENT get binder fail\n");
	}

	vir_bytes binder_node = create_binder(409700);

	size_t send_size = 204800;

	int *data = malloc(send_size);
	int *offset = malloc(send_size);

	time_t  t1 = time(NULL);

	data_g_key = grant_binder(binder_pt,CPF_READ, (vir_bytes) data, send_size);
	offset_g_key = grant_binder(binder_pt, CPF_READ, (vir_bytes) offset, send_size);

	struct binder_write_read bwr;

	bwr.binder_ref = 0;
	bwr.binder_addr = binder_node;
	bwr.code = 0;
	bwr.write_size = 1;
	bwr.read_size = 0;
	bwr.data_size = send_size;
	bwr.offset_size = send_size;
	bwr.data_g_key = data_g_key;
	bwr.offset_g_key = offset_g_key;

	bwr.data = data;
	bwr.offset = offset;

	int r;

	int i;
	int err = 0;
	for(i = 0; i < 100000; i++) {
		bwr.data = data;
		bwr.offset = offset;

		r = binder_write_kernel(&bwr);

		r = binder_read_kernel(&bwr);
	
		if(*((int *)bwr.data) != 100) err++;
	}

	printf("time %d %d\n", time(NULL) - t1, err);

	while(TRUE) {
	
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
