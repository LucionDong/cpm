#include <assert.h>
#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #include "./mqtt/mqtt_msg_handle.h"
#include "device.h"
#include "event/event.h"
#include "manager_adapter_msg.h"
#include "manager_internal.h"
#include "outside_service_manager.h"
#include "outside_service_manager_internal.h"
// #include "rs232_recv.h"
#include "./mcurs232/plugin_frame_handle/parser_rs232_frame.h"
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

    // (*outside_service_manager)->mqtt_relate = calloc(1, sizeof(mqtt_relate_t));
    (*outside_service_manager)->mcurs232_relate = calloc(1, sizeof(mcurs232_relate_t));
    // init_mqtt_relate((*outside_service_manager)->mqtt_relate);
    init_mcurs_relate((*outside_service_manager)->mcurs232_relate);

    return 0;
}

int destroy_all(esv_outside_service_manager_t *outside_service_manager) {
#if 0
    pthread_mutex_lock(&outside_service_manager->mqtt_relate->send_share->mqtt_message_list_mutex);
    pthread_cond_signal(&outside_service_manager->mqtt_relate->recv_share->mqtt_message_list_cond);
    pthread_mutex_unlock(&outside_service_manager->mqtt_relate->send_share->mqtt_message_list_mutex);
    pthread_join(outside_service_manager->mqtt_relate->send_thread, NULL);

    pthread_mutex_destroy(&outside_service_manager->mqtt_relate->send_share->mqtt_message_list_mutex);
    pthread_cond_destroy(&outside_service_manager->mqtt_relate->send_share->mqtt_message_list_cond);
    pthread_mutex_destroy(&outside_service_manager->mqtt_relate->recv_share->mqtt_message_list_mutex);
    pthread_cond_destroy(&outside_service_manager->mqtt_relate->recv_share->mqtt_message_list_cond);
#endif
    // destroy_mqtt_pthread(outside_service_manager->mqtt_relate);

    // destroy_mqtt_share_element(outside_service_manager->mqtt_relate->recv_share);
    // destroy_mqtt_share_element(outside_service_manager->mqtt_relate->send_share);

    // destroy_mqtt_share(outside_service_manager->mqtt_relate);
    // destroy_mqtt_relate(outside_service_manager);

    // destroy_message_list(&outside_service_manager->mqtt_relate->send_share->mqtt_message_list_head);
    // destroy_message_list(&outside_service_manager->mqtt_relate->recv_share->mqtt_message_list_head);

    destroy_mcurs232_pthread(outside_service_manager->mcurs232_relate);
    destroy_mcurs232_share(outside_service_manager->mcurs232_relate);
    destroy_mcurs232_relate(outside_service_manager);

    free(outside_service_manager);

    return 0;
}

esv_outside_service_manager_t *esv_outside_service_manager_create() {
    int rv = 0;
    system("/usr/local/sh/mcu_restart.sh restart");
    // esv_outside_service_manager_t *outside_service_manager = calloc(1, sizeof(esv_outside_service_manager_t));
    esv_outside_service_manager_t *outside_service_manager = NULL;

    init_manager_and_element(&outside_service_manager);
    pthread_create(&outside_service_manager->mcurs232_relate->mcurs232_thread, NULL, mcurs232_thread_func,
                   outside_service_manager);

    nlog_notice("outside service manager start");

    return outside_service_manager;
}

void esv_outside_service_manager_destory(esv_outside_service_manager_t *manager) {
    free(manager);
    nlog_notice("outside service manager exit");
}

void esv_outside_service_manager_set_neu_manager(esv_outside_service_manager_t *outside_service_manager,
                                                 neu_manager_t *manager) {
    outside_service_manager->neu_manager = manager;
}
