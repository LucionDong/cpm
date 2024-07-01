#ifndef _ESV_OUTSIDE_SERVICE_MANAGER_H_
#define _ESV_OUTSIDE_SERVICE_MANAGER_H_

typedef struct esv_outside_service_manager_s esv_outside_service_manager_t;

#include "adapter/driver/device_internal.h"
#include "manager_internal.h"
#include "mcurs232/frame_handle/parser_rs232_frame.h"
#include "mqtt/mqtt_msg_handle.h"
#include "outside_service_manager_common.h"
#include "outside_service_manager_internal.h"

typedef enum esv_outside_service_reqresp_type {
    ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA = 0,
} esv_outside_service_reqresp_type_e;

static const char *esv_outside_service_reqresp_type_string_t[] = {
    [ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA] = "ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA",
};

inline static const char *esv_outside_service_reqresp_type_string(esv_outside_service_reqresp_type_e type) {
    return esv_outside_service_reqresp_type_string_t[type];
}

typedef struct esv_outside_service_reqresp_head {
    esv_outside_service_reqresp_type_e type;
    char sender[64];
    char receiver[64];
} esv_outside_service_reqresp_head_t;

struct esv_outside_service_manager_s {
    neu_manager_t *neu_manager;
    nng_socket socket;
    neu_events_t *events;
    neu_event_io_t *event_io_ctx;

    mqtt_class_t *mqtt_class;

    mcurs232_class_t *mcurs232_class;

#if 0
    MQTTAsync client;
    pthread_t send_thread;
    pthread_t recv_thread;

    pthread_mutex_t mqtt_message_recv_list_mutex;
    pthread_cond_t mqtt_message_recv_list_cond;

    pthread_mutex_t mqtt_message_send_list_mutex;
    pthread_cond_t mqtt_message_send_list_cond;

    mqtt_message_node_t *mqtt_message_recv_list_head;
    mqtt_message_node_t *mqtt_message_send_list_head;
#endif
};
esv_outside_service_manager_t *esv_outside_service_manager_create();
void esv_outside_service_manager_destory(esv_outside_service_manager_t *manager);
void esv_outside_service_manager_set_neu_manager(esv_outside_service_manager_t *outside_service_manager,
                                                 neu_manager_t *manager);
int esv_outside_service_manager_thing_model_msg_send(esv_outside_service_manager_t *outside_service_manager,
                                                     const esv_thing_model_trans_data_inproc_t *msg);

/* int esv_outside_service_manager_dispatch_msg(esv_outside_service_manager_t *outside_service_manager, const
 * esv_between_adapter_driver_msg_t *msg); */
#endif /* ifndef _OUTSIDE_SERVICE_MANAGER_H_ */
