#if 0
#include <assert.h>
#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "event/event.h"
#include "manager_internal.h"
#include "outside_service_manager.h"
#include "outside_service_manager_internal.h"
#include "utils/log.h"

/* static const char *const url = "ipc:///tmp/nng/cpm/esv_outside_service_manager"; */

nng_msg *esv_thing_model_msg_nng_msg_gen(const esv_thing_model_trans_data_inproc_t *data);
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
	/* neu_event_io_param_t param   = { */
        /* .usr_data = (void *) outside_service_manager, */
        /* .cb       = outside_service_manager_loop, */
    /* }; */

	/* outside_service_manager ->events            = neu_event_new(); */

	/* rv = nng_pair1_open(&outside_service_manager->socket); */
	/* assert(rv == 0); */

	/* rv = nng_listen(outside_service_manager->socket, url, NULL, 0); */
	/* assert(rv == 0); */

	/* nng_socket_set_int(outside_service_manager->socket, NNG_OPT_RECVBUF, 8192); */
    /* nng_socket_set_int(outside_service_manager->socket, NNG_OPT_SENDBUF, 8292); */
    /* nng_socket_set_ms(outside_service_manager->socket, NNG_OPT_SENDTIMEO, 1000); */
    /* nng_socket_get_int(outside_service_manager->socket, NNG_OPT_RECVFD, &param.fd); */
	/* outside_service_manager->event_io_ctx = neu_event_add_io(outside_service_manager->events, param); */

	nlog_notice("outside service manager start");

	return outside_service_manager;
}

void esv_outside_service_manager_destory(esv_outside_service_manager_t *manager) {
	/* nng_close(manager->socket); */
	/* neu_event_del_io(manager->events, manager->event_io_ctx); */
	/* neu_event_close(manager->events); */

	free(manager);
	nlog_notice("outside service manager exit");
}

void esv_outside_service_manager_set_neu_manager(esv_outside_service_manager_t *outside_service_manager, neu_manager_t *manager) {
	outside_service_manager->neu_manager = manager;
}

/* int esv_outside_service_manager_thing_model_msg_send(esv_outside_service_manager_t *outside_service_manager, const esv_thing_model_trans_data_inproc_t *msg) { */
/* 	nng_msg *msg_to_send = esv_thing_model_msg_nng_msg_gen(msg); */
/* 	int ret = nng_sendmsg(outside_service_manager->socket, msg_to_send, 0); */
/* 	if (ret != 0) { */
/* 		nng_msg_free(msg_to_send); */
/* 		nlog_error("outside service manager send ipc error: %s", nng_strerror(ret)); */
/* 	} */
/* 	return ret; */
/* } */

/* nng_msg *esv_thing_model_msg_nng_msg_gen(const esv_thing_model_trans_data_inproc_t *data) { */
/*     nng_msg *msg       = NULL; */
/* 	void *body = NULL; */

/* 	/1* char* data_json_str = json_dumps(data->data_root, 0); *1/ */
/* 	size_t data_json_size = json_dumpb(data->data_root, NULL, 0, 0); */
/* 	nng_msg_alloc(&msg, sizeof(esv_outside_service_reqresp_head_t) + sizeof(esv_thing_model_trans_data_ipc_t) + data_json_size); */	
/* 	body = nng_msg_body(msg); */
/* 	esv_outside_service_reqresp_head_t *data_head = calloc(1, sizeof(esv_outside_service_reqresp_head_t)); */

	
/* 	data_head->type = ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA; */ 	
/* 	/1* memcpy(data_head->sender, "core-plugin-manager", strlen("core-plugin-manager") + 1); *1/ */
/* 	/1* memcpy(data_head->receiver, "iot-core-service", strlen("iot-core-service") + 1); *1/ */
/* 	strncpy(data_head->sender, "core-plugin-manager", sizeof(data_head->sender)); */
/* 	strncpy(data_head->receiver, "iot-core-service", sizeof(data_head->receiver)); */

/* 	/1* char *ipc_data_json_str = json_dumps(data->data_root, 0); *1/ */

/* 	esv_thing_model_trans_data_ipc_t *ipc_data = calloc(1, sizeof(esv_thing_model_trans_data_ipc_t)); */
/* 	/1* memcpy(ipc_data->product_key, data->product_key, strlen(data->product_key) + 1); *1/ */
/* 	/1* memcpy(ipc_data->device_name, data->device_name, strlen(data->device_name) + 1); *1/ */
/* 	ipc_data->method = data->method; */
/* 	strncpy(ipc_data->product_key, data->product_key, sizeof(ipc_data->product_key)); */
/* 	strncpy(ipc_data->device_name, data->device_name, sizeof(ipc_data->device_name)); */

/* 	memcpy(body, data_head, sizeof(esv_outside_service_reqresp_head_t)); */
/* 	memcpy((uint8_t *)body + sizeof(esv_outside_service_reqresp_head_t), ipc_data, sizeof(esv_thing_model_trans_data_ipc_t)); */
/* 	/1* strcpy((char *)body + sizeof(esv_outside_service_reqresp_head_t) + sizeof(esv_thing_model_trans_data_ipc_t), ipc_data_json_str); *1/ */
/* 	if (data_json_size > 0) { */
/* 		json_dumpb(data->data_root, (char *)body + sizeof(esv_outside_service_reqresp_head_t) + sizeof(esv_thing_model_trans_data_ipc_t), data_json_size, 0); */
/* 	} */	

/* end: */
/* 	free(data_head); */
/* 	/1* free(ipc_data_json_str); *1/ */
/* 	free(ipc_data); */

/* 	return msg; */
/* } */

/* static int outside_service_manager_loop(enum neu_event_io_type type, int fd, void *usr_data) { */

/* 	esv_outside_service_manager_t *     manager = (esv_outside_service_manager_t *) usr_data; */
/*     int                 rv      = 0; */
/*     nng_msg *           msg     = NULL; */
/*     esv_outside_service_reqresp_head_t *header  = NULL; */

/*     if (type == NEU_EVENT_IO_CLOSED || type == NEU_EVENT_IO_HUP) { */
/*         nlog_warn("outside service manager socket(%d) recv closed or hup %d.", fd, type); */
/*         return 0; */
/*     } */

/*     rv = nng_recvmsg(manager->socket, &msg, NNG_FLAG_NONBLOCK); */
/*     if (rv != 0) { */
/*         nlog_warn("outside service manager recv msg error: %d", rv); */
/*         return 0; */
/*     } */

/*     header = (esv_outside_service_reqresp_head_t *) nng_msg_body(msg); */

/*     nlog_info("outside service manager recv msg from: %s to %s, type: %s", header->sender, */
/*               header->receiver, esv_outside_service_reqresp_type_string(header->type)); */
/*     switch (header->type) { */
/* 		case ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA: { */
/* 		 esv_thing_model_trans_data_ipc_t *cmd = (esv_thing_model_trans_data_ipc_t *) &header[1]; */
/* 		 nlog_info("ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA product_key: %s, device_name: %s", cmd->product_key, cmd->device_name); */
/* 		 nlog_info("thing model json byte: %s", cmd->json_bytes); */
/* 		 forward_thing_model_msg(manager->neu_manager, cmd); */
/* 		 } */
/* 	} */
/* 	nng_msg_free(msg); */
/* 	return 0; */
/* } */


/* int esv_outside_service_manager_dispatch_msg(esv_outside_service_manager_t *outside_service_manager, const esv_between_adapter_driver_msg_t *msg) { */
/* 	/1* TODO:  <24-12-23, winston> */ 
/* 	 * 根据 msg 类型 */
/* 	 * 发送给网关 gatewaybroker mqtt broker */
/* 	 * 发送给网关 mcurs232 */
/* 	 * *1/ */
/* 	return 0; */
/* } */
#endif

#include <assert.h>
#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "./mqtt/mqtt_msg_handle.h"
#include "device.h"
#include "event/event.h"
#include "manager_adapter_msg.h"
#include "manager_internal.h"
#include "outside_service_manager.h"
#include "outside_service_manager_internal.h"
// #include "rs232_recv.h"
#include "utils/log.h"

/* static const char *const url = "ipc:///tmp/nng/cpm/esv_outside_service_manager"; */

nng_msg *esv_thing_model_msg_nng_msg_gen(const esv_thing_model_trans_data_inproc_t *data);
static int outside_service_manager_loop(enum neu_event_io_type type, int fd, void *usr_data);

#define SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#if 0
struct esv_outside_service_manager_s {
    neu_manager_t *neu_manager;
    nng_socket socket;
    neu_events_t *events;
    neu_event_io_t *event_io_ctx;
};
#endif

int init_manager_and_element(esv_outside_service_manager_t **outside_service_manager) {
    *outside_service_manager = calloc(1, sizeof(esv_outside_service_manager_t));
    if (*outside_service_manager == NULL) {
        perror("calloc error");
        return -1;
    }

    (*outside_service_manager)->mqtt_class = calloc(1, sizeof(mqtt_class_t));
    (*outside_service_manager)->mcurs232_class = calloc(1, sizeof(mcurs232_class_t));
    init_mqtt_class((*outside_service_manager)->mqtt_class);
    init_mcurs_class((*outside_service_manager)->mcurs232_class);

    return 0;
}

int destroy_all(esv_outside_service_manager_t *outside_service_manager) {
#if 0
    pthread_mutex_lock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    pthread_cond_signal(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_cond);
    pthread_mutex_unlock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    pthread_join(outside_service_manager->mqtt_class->send_thread, NULL);

    pthread_mutex_destroy(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    pthread_cond_destroy(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_cond);
    pthread_mutex_destroy(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_mutex);
    pthread_cond_destroy(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_cond);
#endif
    destroy_mqtt_pthread(outside_service_manager->mqtt_class);

    destroy_mqtt_share_element(outside_service_manager->mqtt_class->recv_share);
    destroy_mqtt_share_element(outside_service_manager->mqtt_class->send_share);

    destroy_mqtt_share(outside_service_manager->mqtt_class);
    destroy_mqtt_class(outside_service_manager);

    destroy_message_list(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_head);
    destroy_message_list(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_head);

    destroy_mcurs232_pthread(outside_service_manager->mcurs232_class);
    destroy_mcurs232_share(outside_service_manager->mcurs232_class);
    destroy_mcurs232_class(outside_service_manager);

    free(outside_service_manager);

    return 0;
}

esv_outside_service_manager_t *esv_outside_service_manager_create() {
    int rv = 0;
    system("/usr/local/sh/mcu_restart.sh restart");
    esv_outside_service_manager_t *outside_service_manager = calloc(1, sizeof(esv_outside_service_manager_t));

    init_manager_and_element(&outside_service_manager);
    pthread_create(&outside_service_manager->mqtt_class->recv_thread, NULL, recv_thread_func, outside_service_manager);
    pthread_create(&outside_service_manager->mqtt_class->send_thread, NULL, send_thread_func, outside_service_manager);

    pthread_create(&outside_service_manager->mcurs232_class->mcurs232_thread, NULL, mcurs232_thread_func,
                   outside_service_manager);

    /* neu_event_io_param_t param   = { */
    /* .usr_data = (void *) outside_service_manager, */
    /* .cb       = outside_service_manager_loop, */
    /* }; */

    /* outside_service_manager ->events            = neu_event_new(); */

    /* rv = nng_pair1_open(&outside_service_manager->socket); */
    /* assert(rv == 0); */

    /* rv = nng_listen(outside_service_manager->socket, url, NULL, 0); */
    /* assert(rv == 0); */

    /* nng_socket_set_int(outside_service_manager->socket, NNG_OPT_RECVBUF, 8192); */
    /* nng_socket_set_int(outside_service_manager->socket, NNG_OPT_SENDBUF, 8292); */
    /* nng_socket_set_ms(outside_service_manager->socket, NNG_OPT_SENDTIMEO, 1000); */
    /* nng_socket_get_int(outside_service_manager->socket, NNG_OPT_RECVFD, &param.fd); */
    /* outside_service_manager->event_io_ctx = neu_event_add_io(outside_service_manager->events, param); */

    nlog_notice("outside service manager start");

    return outside_service_manager;
}

void esv_outside_service_manager_destory(esv_outside_service_manager_t *manager) {
    /* nng_close(manager->socket); */
    /* neu_event_del_io(manager->events, manager->event_io_ctx); */
    /* neu_event_close(manager->events); */

    free(manager);
    nlog_notice("outside service manager exit");
}

void esv_outside_service_manager_set_neu_manager(esv_outside_service_manager_t *outside_service_manager,
                                                 neu_manager_t *manager) {
    outside_service_manager->neu_manager = manager;
}

/* int esv_outside_service_manager_thing_model_msg_send(esv_outside_service_manager_t *outside_service_manager, const
 * esv_thing_model_trans_data_inproc_t *msg) { */
/*      nng_msg *msg_to_send = esv_thing_model_msg_nng_msg_gen(msg); */
/*      int ret = nng_sendmsg(outside_service_manager->socket, msg_to_send, 0); */
/*      if (ret != 0) { */
/*              nng_msg_free(msg_to_send); */
/*              nlog_error("outside service manager send ipc error: %s", nng_strerror(ret)); */
/*      } */
/*      return ret; */
/* } */

/* nng_msg *esv_thing_model_msg_nng_msg_gen(const esv_thing_model_trans_data_inproc_t *data) { */
/*     nng_msg *msg       = NULL; */
/*      void *body = NULL; */

/*      /1* char* data_json_str = json_dumps(data->data_root, 0); *1/ */
/*      size_t data_json_size = json_dumpb(data->data_root, NULL, 0, 0); */
/*      nng_msg_alloc(&msg, sizeof(esv_outside_service_reqresp_head_t) + sizeof(esv_thing_model_trans_data_ipc_t) +
 * data_json_size); */
/*      body = nng_msg_body(msg); */
/*      esv_outside_service_reqresp_head_t *data_head = calloc(1, sizeof(esv_outside_service_reqresp_head_t)); */

/*      data_head->type = ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA; */
/*      /1* memcpy(data_head->sender, "core-plugin-manager", strlen("core-plugin-manager") + 1); *1/ */
/*      /1* memcpy(data_head->receiver, "iot-core-service", strlen("iot-core-service") + 1); *1/ */
/*      strncpy(data_head->sender, "core-plugin-manager", sizeof(data_head->sender)); */
/*      strncpy(data_head->receiver, "iot-core-service", sizeof(data_head->receiver)); */

/*      /1* char *ipc_data_json_str = json_dumps(data->data_root, 0); *1/ */

/*      esv_thing_model_trans_data_ipc_t *ipc_data = calloc(1, sizeof(esv_thing_model_trans_data_ipc_t)); */
/*      /1* memcpy(ipc_data->product_key, data->product_key, strlen(data->product_key) + 1); *1/ */
/*      /1* memcpy(ipc_data->device_name, data->device_name, strlen(data->device_name) + 1); *1/ */
/*      ipc_data->method = data->method; */
/*      strncpy(ipc_data->product_key, data->product_key, sizeof(ipc_data->product_key)); */
/*      strncpy(ipc_data->device_name, data->device_name, sizeof(ipc_data->device_name)); */

/*      memcpy(body, data_head, sizeof(esv_outside_service_reqresp_head_t)); */
/*      memcpy((uint8_t *)body + sizeof(esv_outside_service_reqresp_head_t), ipc_data,
 * sizeof(esv_thing_model_trans_data_ipc_t)); */
/*      /1* strcpy((char *)body + sizeof(esv_outside_service_reqresp_head_t) + sizeof(esv_thing_model_trans_data_ipc_t),
 * ipc_data_json_str); *1/ */
/*      if (data_json_size > 0) { */
/*              json_dumpb(data->data_root, (char *)body + sizeof(esv_outside_service_reqresp_head_t) +
 * sizeof(esv_thing_model_trans_data_ipc_t), data_json_size, 0); */
/*      } */

/* end: */
/*      free(data_head); */
/*      /1* free(ipc_data_json_str); *1/ */
/*      free(ipc_data); */

/*      return msg; */
/* } */

/* static int outside_service_manager_loop(enum neu_event_io_type type, int fd, void *usr_data) { */

/*      esv_outside_service_manager_t *     manager = (esv_outside_service_manager_t *) usr_data; */
/*     int                 rv      = 0; */
/*     nng_msg *           msg     = NULL; */
/*     esv_outside_service_reqresp_head_t *header  = NULL; */

/*     if (type == NEU_EVENT_IO_CLOSED || type == NEU_EVENT_IO_HUP) { */
/*         nlog_warn("outside service manager socket(%d) recv closed or hup %d.", fd, type); */
/*         return 0; */
/*     } */

/*     rv = nng_recvmsg(manager->socket, &msg, NNG_FLAG_NONBLOCK); */
/*     if (rv != 0) { */
/*         nlog_warn("outside service manager recv msg error: %d", rv); */
/*         return 0; */
/*     } */

/*     header = (esv_outside_service_reqresp_head_t *) nng_msg_body(msg); */

/*     nlog_info("outside service manager recv msg from: %s to %s, type: %s", header->sender, */
/*               header->receiver, esv_outside_service_reqresp_type_string(header->type)); */
/*     switch (header->type) { */
/*              case ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA: { */
/*               esv_thing_model_trans_data_ipc_t *cmd = (esv_thing_model_trans_data_ipc_t *) &header[1]; */
/*               nlog_info("ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA product_key: %s, device_name: %s",
 * cmd->product_key, cmd->device_name); */
/*               nlog_info("thing model json byte: %s", cmd->json_bytes); */
/*               forward_thing_model_msg(manager->neu_manager, cmd); */
/*               } */
/*      } */
/*      nng_msg_free(msg); */
/*      return 0; */
/* } */

int esv_outside_service_manager_dispatch_msg(esv_outside_service_manager_t *outside_service_manager,
                                             const esv_between_adapter_driver_msg_t *msg) {
    /* TODO:  <24-12-23, winston>
     * 根据 msg 类型
     * 发送给网关 gatewaybroker mqtt broker
     * 发送给网关 mcurs232
     * */
    nlog_info("esv_outside_service_manager_dispatch_msg");
    if (msg->msg_type == ESV_TAM_BYTES_PTR) {
        // send to mcurs232
        // unknow msg size for msg->msg
        nlog_info("esv_outside_service_manager_dispatch_msg byte");
        printf("esv_outside_service_manager_dispatch_msg after\n");
        // hnlog_notice(msg->msg, msg->msg_len);
        push_back_serial_port_read_buf_and_check(outside_service_manager->mcurs232_class, (unsigned char *) msg->msg,
                                       msg->msg_len);
    } else if (msg->msg_type == ESV_TAM_JSON_OBJECT_PTR) {
        // send to mqtt broker
        append_node_to_send_list(outside_service_manager, (char *) msg->msg, PLUGIN_TO_MQTT);
        nlog_info("msg->msg_type == ESV_TAM_JSON_OBJECT_PTR");
    }
    return 0;
}
