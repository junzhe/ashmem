#define ASHMEM_MESSAGE "Ashmem called\n"
#define MAX_ASHMEM_N 1024
#define ASHMEM_NAME_PREFIX "dev/ashmem/"
#define ASHMEM_NAME_PREFIX_LEN (sizeof(ASHMEM_NAME_PREFIX) - 1)
#define ASHMEM_FULL_NAME_LEN (ASHMEM_NAME_LEN + ASHMEM_NAME_PREFIX_LEN)

#include "ashmem.h"

struct ashmem_struct {
	char name[ASHMEM_FULL_NAME_LEN];
	int id;
	struct shmid_ds shmid_ds;
	vir_bytes page;
	int vm_id;
}; 

static struct ashmem_struct ashmem_list[MAX_ASHMEM_N];
static int ashmem_list_n = 0;

/* Entry points to the ashmem driver. */
static struct chardriver ashmem_tab =
{
	ashmem_open,
	ashmem_close,
	ashmem_ioctl,
	ashmem_prepare,
	ashmem_transfer,
	ashmem_cleanup,
	ashmem_alarm,
	ashmem_cancel,
	ashmem_select,
	ashmem_other	
};

/** Represents the /dev/ashmem device. */
static struct device ashmem_device;

static int ashmem_open(message *m)
{
	printf("call ashmem_open\n");
	printf("%d %d %d\n", m->ADDRESS, m->POSITION, m->HIGHPOS);
	return OK;
}

static int ashmem_close(message *m)
{
	printf("call ashmem_close\n");
	return OK;
}

static struct device * ashmem_prepare(dev_t device)
{
	ashmem_device.dv_base = make64(0, 0);
    	ashmem_device.dv_size = make64(strlen(ASHMEM_MESSAGE), 0);
	return &ashmem_device;
}	
static int ashmem_transfer(endpoint_t endpt, int opcode, u64_t position,
        iovec_t *iov, unsigned int nr_req, endpoint_t user_endpt, unsigned int
	flags)
{
	printf("call ashmem_transfer\n");

        int bytes, ret;

	if (nr_req != 1)
	{
	/* This should never trigger for character drivers at the moment. */
		printf("ASHMEM: vectored transfer request, using first element only\n");
	}   

	bytes = strlen(ASHMEM_MESSAGE) - ex64lo(position) < iov->iov_size ?
	strlen(ASHMEM_MESSAGE) - ex64lo(position) : iov->iov_size;

	if (bytes <= 0)
	{
		return OK;
	}

	switch (opcode)
 	{
		case DEV_GATHER_S:
			ret = sys_safecopyto(endpt, (cp_grant_id_t) iov->iov_addr, 0,
			(vir_bytes) (ASHMEM_MESSAGE + ex64lo(position)),bytes);
		      	iov->iov_size -= bytes;
		       	break;

		default:
			return EINVAL;
	}

	return ret;
}

static int ashmem_ioctl(message *m)
{
	printf("call ashmem_ioctl\n");

	printf("%d, %d, %d\n", m->COUNT, m->REQUEST, ASHMEM_IOC_SET_NAME);

	char t[10];

	sys_safecopyfrom(VFS_PROC_NR, (cp_grant_id_t) m->IO_GRANT, (vir_bytes) 0, (vir_bytes) t, 4);

	printf("%s\n", t);
}

static void ashmem_cleanup(void)
{
	printf("call ashmem_cleanup\n");
}

static void ashmem_alarm(message *m)
{
	printf("call ashmem_alarm\n");
}

static int ashmem_cancel(message *m)
{
	printf("call ashmem_cancel\n");
}

static int ashmem_select(message *m)
{
	printf("call ashmem_select\n");
}

static int ashmem_other(message *m)
{
	printf("ashmem_other %d\n", (m->m_type == ASHMEM_CREATE));

	return OK;
}

static int sef_cb_lu_state_save(int UNUSED(state))
{
	return OK;
}

static int lu_state_restore()
{

    return OK;
}

static void sef_local_startup()
{
	/*
 	 * Register init callbacks. Use the same function for all event types
 	 */
	sef_setcb_init_fresh(sef_cb_init);
 	sef_setcb_init_lu(sef_cb_init);
	sef_setcb_init_restart(sef_cb_init);

	/*
	 * Register live update callbacks.
	 */
	/* - Agree to update immediately when LU is requested in a valid state. */
	sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
	/* - Support live update starting from any standard state. */
	sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
	/* - Register a custom routine to save the state. */
	sef_setcb_lu_state_save(sef_cb_lu_state_save);

	/* Let SEF perform startup. */
 	sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
	/* Initialize the hello driver. */
	int do_announce_driver = TRUE;

	switch(type) {
		case SEF_INIT_FRESH:
			printf("%s", ASHMEM_MESSAGE);
		break;

		case SEF_INIT_LU:
			/* Restore the state. */
			lu_state_restore();
			do_announce_driver = FALSE;

			printf("%sHey, I'm a new version!\n", ASHMEM_MESSAGE);
		break;
		case SEF_INIT_RESTART:
	     		printf("%sHey, I've just been restarted!\n", ASHMEM_MESSAGE);
		break;
	}


	/* Announce we are up when necessary. */
	if (do_announce_driver) {
		chardriver_announce();
	}	

	/* Initialization completed successfully. */
	return OK;
}

int main(int argc, char *argv[])
{
	/* SEF local startup. */
	sef_local_startup();
	chardriver_task(&ashmem_tab, CHARDRIVER_SYNC);

	return OK;
}
