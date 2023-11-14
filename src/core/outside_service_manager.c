#include <assert.h>
#include <stdlib.h>

#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>

#include "event/event.h"
#include "manager_thing_model.h"
#include "outside_service_manager_internal.h"
#include "utils/log.h"
#include "manager_internal.h"
#include "outside_service_manager.h"

static const char *const url = "ipc:///tmp/nng/cpm/esv_outside_service_manager";

static int outside_service_manager_loop(enum neu_event_io_type type, int fd, void *usr_data);

struct esv_outside_service_manager_s {
	neu_manager_t * neu_manager;
	nng_socket socket;
	neu_events_t *  events;
	neu_event_io_t *event_io_ctx;
};

esv_outside_service_manager_t *esv_outside_service_manager_create() {
	int rv = 0;
	esv_outside_service_manager_t *outside_service_manager = calloc(1, sizeof(esv_outside_service_manager_t));
	neu_event_io_param_t param   = {
        .usr_data = (void *) outside_service_manager,
        .cb       = outside_service_manager_loop,
    };

	outside_service_manager ->events            = neu_event_new();

	rv = nng_pair1_open(&outside_service_manager->socket);
	assert(rv == 0);

	rv = nng_listen(outside_service_manager->socket, url, NULL, 0);
	assert(rv == 0);

	nng_socket_set_int(outside_service_manager->socket, NNG_OPT_RECVBUF, 8192);
    nng_socket_set_int(outside_service_manager->socket, NNG_OPT_SENDBUF, 8292);
    nng_socket_set_ms(outside_service_manager->socket, NNG_OPT_SENDTIMEO, 1000);
    nng_socket_get_int(outside_service_manager->socket, NNG_OPT_RECVFD, &param.fd);
	outside_service_manager->event_io_ctx = neu_event_add_io(outside_service_manager->events, param);

	nlog_notice("outside service manager start");

	return outside_service_manager;
}

void esv_outside_service_manager_destory(esv_outside_service_manager_t *manager) {
	nng_close(manager->socket);
	neu_event_del_io(manager->events, manager->event_io_ctx);
	neu_event_close(manager->events);

	free(manager);
	nlog_notice("outside service manager exit");
}

void esv_outside_service_set_neu_manager(esv_outside_service_manager_t *outside_service_manager, neu_manager_t *manager) {
	outside_service_manager->neu_manager = manager;
}

static int outside_service_manager_loop(enum neu_event_io_type type, int fd, void *usr_data) {

	esv_outside_service_manager_t *     manager = (esv_outside_service_manager_t *) usr_data;
    int                 rv      = 0;
    nng_msg *           msg     = NULL;
    esv_outside_service_reqresp_head_t *header  = NULL;

    if (type == NEU_EVENT_IO_CLOSED || type == NEU_EVENT_IO_HUP) {
        nlog_warn("outside service manager socket(%d) recv closed or hup %d.", fd, type);
        return 0;
    }

    rv = nng_recvmsg(manager->socket, &msg, NNG_FLAG_NONBLOCK);
    if (rv != 0) {
        nlog_warn("outside service manager recv msg error: %d", rv);
        return 0;
    }

    header = (esv_outside_service_reqresp_head_t *) nng_msg_body(msg);

    nlog_info("outside service manager recv msg from: %s to %s, type: %s", header->sender,
              header->receiver, esv_outside_service_reqresp_type_string(header->type));
    switch (header->type) {
		case ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA: {
		 esv_thing_model_trans_data_ipc_t *cmd = (esv_thing_model_trans_data_ipc_t *) &header[1];
		 nlog_info("ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA product_key: %s, device_name: %s", cmd->product_key, cmd->device_name);
		 nlog_info("thing model json str: %s", cmd->json_str);
		 forward_thing_model_msg(manager->neu_manager, cmd);
		 }
	}
	nng_msg_free(msg);
	return 0;
}
