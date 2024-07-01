/*
 * mqtt_conn.h
 * Copyright (C) 2023 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __MQTT_CONN_H__
#define __MQTT_CONN_H__

typedef struct mqtt_class mqtt_class_t;

#include <MQTTAsync.h>

#include "../outside_service_manager.h"
// #include "outside_service_manager_internal.h"

#define ADDRESS "tcp://127.0.0.1:1883"

#define CLIENTID "Client_dfefadeasemore"
// #define RECV_TOPIC "Dong_Test/1"
#define RECV_TOPIC "/sys/gatewaybrokercore/emlocalmqttt/thing/event/property/post"
// #define SEND_TOPIC "Dong_Send_Test/1"
#define SEND_TOPIC "/sys/esvcpm/mqtt/thing/event/property/post"
#define PAYLOAD "Hello"
#define QOS 1

static int disc_finished = 0;
static int subscribed = 0;
static int finished = 0;

typedef enum mqtt_message_type {
    MANAGER_TO_PLUGIN = 1,
    PLUGIN_TO_MQTT,
    PLUGIN_TO_ADAPTER,
} mqtt_message_type_e;

typedef struct mqtt_message_node {
    char *message;
    mqtt_message_type_e message_type;
    struct mqtt_message_node *next, *prev;
} mqtt_message_node_t;

typedef struct mqtt_share {
    pthread_mutex_t mqtt_message_list_mutex;
    pthread_cond_t mqtt_message_list_cond;
    mqtt_message_node_t *mqtt_message_list_head;
} mqtt_share_t;

typedef struct mqtt_class {
    MQTTAsync client;
    pthread_t send_thread;
    pthread_t recv_thread;

    mqtt_share_t *send_share;
    mqtt_share_t *recv_share;
    char *client_id;
} mqtt_class_t;

void onConnect(void *context, MQTTAsync_successData *response);
void onConnectFailure(void *context, MQTTAsync_failureData *response);
void onSubscribe(void *context, MQTTAsync_successData *response);
void onSubscribeFailure(void *context, MQTTAsync_failureData *response);
void connlost(void *context, char *cause);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void onReconnected(void *context, char *cause);
// int destroy_all(esv_outside_service_manager_t *outside_service_manager);
int init_manager_and_element(esv_outside_service_manager_t **outside_service_manager);
void *send_thread_func(void *arg);
void *recv_thread_func(void *arg);
void onDisconnectFailure(void *context, MQTTAsync_failureData *response);
void onDisconnect(void *context, MQTTAsync_successData *response);
void onConnect(void *context, MQTTAsync_successData *response);
int init_mqtt_class(mqtt_class_t *mqtt_class);
int init_share(mqtt_share_t *share);
int destroy_mqtt_class(esv_outside_service_manager_t *outside_service_manager);
int destroy_mqtt_share(mqtt_class_t *mqtt_class);
int destroy_mqtt_share_element(mqtt_share_t *share);
int destroy_message_list(mqtt_message_node_t **head);
int destroy_mqtt_pthread(mqtt_class_t *mqtt_class);
int get_time_stamp(time_t *timestamp);
int composition_timestamp(const time_t *timestamp, char *topic_id);
int append_node_to_send_list(esv_outside_service_manager_t *outside_service_manager, const char *message,
                             mqtt_message_type_e message_type);
#endif /* !__MQTT_CONN_H__ */
