/*
 * mqtt_conn.h
 * Copyright (C) 2023 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __MQTT_CONN_H__
#define __MQTT_CONN_H__

#include "outside_service_manager.h"
// #include "outside_service_manager_internal.h"

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
int init_mqtt_class(Mqtt_class_t *mqtt_class);
int init_share(mqtt_share_t *share);
int destroy_mqtt_class(esv_outside_service_manager_t *outside_service_manager);
int destroy_mqtt_share(Mqtt_class_t *mqtt_class);
int destroy_mqtt_share_element(mqtt_share_t *share);
int destroy_message_list(mqtt_message_node_t **head);
int destroy_mqtt_pthread(Mqtt_class_t *mqtt_class);
int get_time_stamp(time_t *timestamp);
int composition_timestamp(const time_t *timestamp, char *topic_id);
int append_node_to_send_list(esv_outside_service_manager_t *outside_service_manager, const char *message,
                             mqtt_message_type_e message_type);
#endif /* !__MQTT_CONN_H__ */
