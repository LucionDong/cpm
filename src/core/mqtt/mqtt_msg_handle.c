/*
 *
 * Copyright (C) 2023-12-20 09:07 dongbin <dongbin0625@163.com>
 *
 */

#include <MQTTAsync.h>
#include <MQTTClient.h>
#include <MQTTClientPersistence.h>
#include <MQTTSubscribeOpts.h>
// #include <utils/log.h>
#include "utils/log.h"
#include "utils/utlist.h"
// #include <esvcpm/utils/log.h>
// #include <esvcpm/utils/utlist.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "device.h"
#include "manager_adapter_msg.h"
#include "mqtt_conn.h"
#include "outside_service_manager.h"
#include "outside_service_manager_internal.h"

int move_all_list_node(mqtt_share_t *share, mqtt_message_node_t **tmp_head);
int pop_list(mqtt_message_node_t **head);
int make_mqtt_send_to_plugin_msg(esv_between_adapter_driver_msg_t *send_to_plugin_msg, const char *message);
void onSubscribe5(void *context, MQTTAsync_successData5 *response);

int get_time_stamp(time_t *timestamp) {
    struct tm timeinfo;
    *timestamp = time(NULL);

    localtime_r(timestamp, &timeinfo);
    return 0;
}

int composition_timestamp(const time_t *timestamp, char *client_id) {
    sprintf(client_id, "%s%ld", client_id, *timestamp);
    nlog_info("====client_id %s\n", client_id);
    return 0;
}

void onReconnected(void *context, char *cause) {
    // MQTTAsync client = (MQTTAsync) context;
    Mqtt_class_t *mqtt_class = (Mqtt_class_t *) context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    nlog_info("Successful reconnection");

    nlog_info(
        "Subscribing to topic %s\nfor client %s using QoS%d\n\n"
        "Press Q<Enter> to quit\n\n",
        RECV_TOPIC, mqtt_class->client_id, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = mqtt_class->client;
    nlog_info("mqtt_client_id %s", mqtt_class->client_id);
    if ((rc = MQTTAsync_subscribe(mqtt_class->client, RECV_TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

void onReconnected5(void *context, char *cause) {
    nlog_info("onReconnected5");
    Mqtt_class_t *mqtt_class = (Mqtt_class_t *) context;

    MQTTSubscribe_options subscribe_options = MQTTSubscribe_options_initializer;
    MQTTAsync_callOptions copts = MQTTAsync_callOptions_initializer;

    subscribe_options.noLocal = 1;
    copts.subscribeOptions = subscribe_options;
    copts.context = context;
    // copts.onSuccess5 = onSubscribe5;

    int rc;
    if ((rc = MQTTAsync_subscribe(mqtt_class->client, RECV_TOPIC, QOS, &copts)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to start subscribe, return code :%d", rc);
        finished = 1;
    }
}

void onConnect(void *context, MQTTAsync_successData *response) {
    Mqtt_class_t *mqtt_class = (Mqtt_class_t *) context;
    // MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    nlog_info("Successful connection\n");

    nlog_info(
        "Subscribing to topic %s\nfor client %s using QoS%d\n\n"
        "Press Q<Enter> to quit\n\n",
        RECV_TOPIC, mqtt_class->client_id, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = mqtt_class->client;
    if ((rc = MQTTAsync_subscribe(mqtt_class->client, RECV_TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

void connlost(void *context, char *cause) {
    MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    nlog_info("\nConnection lost\n");
    if (cause)
        nlog_info("     cause: %s\n", cause);

    nlog_info("Reconnecting");
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    esv_outside_service_manager_t *outside_service_manager = (esv_outside_service_manager_t *) context;
    int count = 0;
    mqtt_message_node_t *elt;
    nlog_info("Message arrived\n");
    nlog_info("     topic: %s\n", topicName);
    nlog_info("  -- message: %.*s\n", message->payloadlen, (char *) message->payload);

    pthread_mutex_lock(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_mutex);
    nlog_info("msgarrvd lock\n");
    // DL_COUNT(plugin->mqtt_message_recv_list_head, elt, count);
    // if (count < 10) {
    mqtt_message_node_t *new_node = calloc(1, sizeof(mqtt_message_node_t));
    new_node->message = malloc(sizeof(char) * (message->payloadlen + 1));
    memcpy(new_node->message, message->payload, message->payloadlen);
    // new_node->message = strdup(message->payload);
    new_node->message_type = MANAGER_TO_PLUGIN;
    DL_APPEND(outside_service_manager->mqtt_class->recv_share->mqtt_message_list_head, new_node);
    // }
    // pthread_mutex_unlock(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_mutex);
    nlog_info("  -- message: %.*s\n", message->payloadlen, (char *) message->payload);

    //将链表中的取到发送链表中
    // pthread_mutex_lock(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_mutex);
    append_node_to_send_list(outside_service_manager,
                             outside_service_manager->mqtt_class->recv_share->mqtt_message_list_head->message,
                             outside_service_manager->mqtt_class->recv_share->mqtt_message_list_head->message_type);
    pop_list(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_head);
    pthread_mutex_unlock(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_mutex);
    nlog_info("msgarrvd unlock");

    // MQTTAsync_freeMessage(&message);
    // MQTTAsync_free(topicName);
    return 1;
}

void onDisconnectFailure(void *context, MQTTAsync_failureData *response) {
    nlog_warn("Disconnect failed, rc %d\n", response->code);
    disc_finished = 1;
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    nlog_warn("Successful disconnection\n");
    disc_finished = 1;
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    nlog_warn("Subscribe succeeded\n");
    subscribed = 1;
}

void onSubscribe5(void *context, MQTTAsync_successData5 *response) {
    nlog_warn("Subscribe5 succeeded\n");
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    nlog_warn("Subscribe failed, rc %d\n", response->code);
    finished = 1;
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    nlog_warn("Connect failed, rc %d\n", response->code);
    finished = 1;
}

void *send_thread_func(void *arg) {
    esv_outside_service_manager_t *outside_service_manager = (esv_outside_service_manager_t *) arg;
    // neu_plugin_t *plugin = (neu_plugin_t *) arg;

    // MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    int count = 0;
    mqtt_message_node_t *elt;

    //     pubmsg.payload = "hello world";
    //     pubmsg.payloadlen = (int) strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    mqtt_message_node_t *tmp_message_node_head;
    while (1 && disc_finished != 1) {
        tmp_message_node_head = NULL;
        // move_all_list_node(plugin, &tmp_message_node_head);
        pthread_mutex_lock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
        DL_COUNT(outside_service_manager->mqtt_class->send_share->mqtt_message_list_head, elt, count);

        while (count < 1) {
            nlog_info("while count < 1");
            pthread_cond_wait(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_cond,
                              &outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
            DL_COUNT(outside_service_manager->mqtt_class->send_share->mqtt_message_list_head, elt, count);
            // printf("count: %d\n", count);
        }
        nlog_info("count: %d wait over\n", count);
        pthread_mutex_unlock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);

        // move_all_list_node(&outside_service_manager->mqtt_message_send_list_head, &tmp_message_node_head,
        // &outside_service_manager->mqtt_message_send_list_mutex);
        move_all_list_node(outside_service_manager->mqtt_class->send_share, &tmp_message_node_head);
        DL_COUNT(outside_service_manager->mqtt_class->send_share->mqtt_message_list_head, elt, count);
        nlog_info("count: %d wiat over\n", count);
        while (tmp_message_node_head) {
            if (tmp_message_node_head->message_type == MANAGER_TO_PLUGIN) {
                esv_between_adapter_driver_msg_t send_to_plugin_msg;
                make_mqtt_send_to_plugin_msg(&send_to_plugin_msg, tmp_message_node_head->message);
                nlog_info("forward_msg_to_esvdriver");
                forward_msg_to_esvdriver(outside_service_manager->neu_manager, &send_to_plugin_msg);

                if (send_to_plugin_msg.msg) {
                    free(send_to_plugin_msg.msg);
                    send_to_plugin_msg.msg = NULL;
                }
                nlog_info("************forward_msg_to_esvdriver over\n");
            } else if (tmp_message_node_head->message_type == PLUGIN_TO_MQTT) {
                nlog_info("MQTT sendMessage");
                pubmsg.payload = tmp_message_node_head->message;
                pubmsg.payloadlen = strlen(tmp_message_node_head->message);
                nlog_info("len: %d", pubmsg.payloadlen);
                nlog_info("pubmsg.payload: %s", tmp_message_node_head->message);
                if ((rc = MQTTAsync_sendMessage(outside_service_manager->mqtt_class->client, SEND_TOPIC, &pubmsg,
                                                NULL)) != MQTTASYNC_SUCCESS) {
                    nlog_warn("Failed to start sendMessage, return code %d\n", rc);
                }

                esv_between_adapter_driver_msg_t send_to_plugin_msg;
                make_mqtt_send_to_plugin_msg(&send_to_plugin_msg, tmp_message_node_head->message);
                forward_msg_to_esvdriver(outside_service_manager->neu_manager, &send_to_plugin_msg);

                if (send_to_plugin_msg.msg) {
                    free(send_to_plugin_msg.msg);
                    send_to_plugin_msg.msg = NULL;
                }
                nlog_info("MQTT sendMessage over");
            }
            //  pubmsg.payload = tmp_message_node_head->message;
            // pubmsg.payloadlen = strlen(tmp_message_node_head->message);
            //
            //  nlog_info("tmp_message_node_head: %s\n", (char *) pubmsg.payload);
            //             send_to_plugin_msg.method = ESV_TO_ADAPTER_MQTT_PROPERTY_POST;
            // send_to_plugin_msg.msg_type = ESV_TAM_JSON_OBJECT_PTR;
            //             send_to_plugin_msg.msg = strdup(tmp_message_node_head->message);
            //
            // nlog_info("mqtt forward_msg_to_esvdriver start");
            // // if()
            pop_list(&tmp_message_node_head);
            nlog_info("pop_list");
        }
        nlog_info("while over");
    }
    pthread_exit(NULL);
}

int composition_mqtt_send_to_plugin_msg(esv_between_adapter_driver_msg_t *send_to_plugin_msg, const char *message) {
    send_to_plugin_msg->method = ESV_TO_ADAPTER_MQTT_PROPERTY_POST;
    send_to_plugin_msg->msg_type = ESV_TAM_JSON_OBJECT_PTR;
    send_to_plugin_msg->msg = strdup(message);

    return 0;
}

int pop_list(mqtt_message_node_t **head) {
    int count;
    mqtt_message_node_t *elt;
    DL_COUNT(*head, elt, count);
    if (count < 1) {
        return -1;
    }

    mqtt_message_node_t *del_node = *head;
    DL_DELETE(*head, del_node);
    if (del_node->message)
        free(del_node->message);
    free(del_node);
    return 0;
}

int append_node_to_send_list(esv_outside_service_manager_t *outside_service_manager, const char *message,
                             mqtt_message_type_e message_type) {
    if (message == NULL) {
        perror("message is NULL");
        return -1;
    }

    nlog_info("append_node_to_send_list");
    pthread_mutex_lock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    mqtt_message_node_t *new_send_list_node = calloc(1, sizeof(mqtt_message_node_t));
    new_send_list_node->message = strdup(message);
    new_send_list_node->message_type = message_type;
    DL_APPEND(outside_service_manager->mqtt_class->send_share->mqtt_message_list_head, new_send_list_node);
    pthread_cond_signal(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_cond);
    nlog_info("$$$$$$$$$$$$$$$$$$$pthread_cond_signal\n");
    // pthread_cond_signal(&outside_service_manager->mqtt_message_send_list_cond);
    // printf("signal over\n");
    pthread_mutex_unlock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    // pthread_mutex_unlock(&outside_service_manager->mqtt_message_send_list_mutex);

    return 0;
}

// int move_all_list_node(mqtt_message_node_t **list_head, mqtt_message_node_t **tmp_head, pthread_mutex_t *mutex) {
int move_all_list_node(mqtt_share_t *share, mqtt_message_node_t **tmp_head) {
    int count;
    mqtt_message_node_t *elt;
    // pthread_mutex_lock(&(*share)->mqtt_message_list_mutex);
    pthread_mutex_lock(&share->mqtt_message_list_mutex);
    DL_COUNT(share->mqtt_message_list_head, elt, count);
    // DL_COUNT((*share)->mqtt_message_list_head, elt, count);
    if (count < 1) {
        return -1;
    }

    *tmp_head = share->mqtt_message_list_head;
    share->mqtt_message_list_head = NULL;

    // *tmp_head = *list_head;
    // *list_head = NULL;
    pthread_mutex_unlock(&share->mqtt_message_list_mutex);
    return 0;
}

int destroy_message_list(mqtt_message_node_t **head) {
    if (*head == NULL) {
        return 0;
    }
    mqtt_message_node_t *elt, *tmp;
    DL_FOREACH_SAFE(*head, elt, tmp) {
        DL_DELETE(*head, elt);
        if (elt->message)
            free(elt->message);
        free(elt);
    }

    *head = NULL;
    return 0;
}

#if 0
int destroy_all(esv_outside_service_manager_t *outside_service_manager) {
    pthread_mutex_lock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    pthread_cond_signal(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_cond);
    pthread_mutex_unlock(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    pthread_join(outside_service_manager->mqtt_class->send_thread, NULL);

    pthread_mutex_destroy(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_mutex);
    pthread_cond_destroy(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_cond);
    pthread_mutex_destroy(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_mutex);
    pthread_cond_destroy(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_cond);

    destroy_list(&outside_service_manager->mqtt_class->send_share->mqtt_message_list_head);
    destroy_list(&outside_service_manager->mqtt_class->recv_share->mqtt_message_list_head);

    if (outside_service_manager->mqtt_class->recv_share) {
        free(outside_service_manager->mqtt_class->recv_share);
    }

    if (outside_service_manager->mqtt_class->send_share) {
        free(outside_service_manager->mqtt_class->send_share);
    }

    if (outside_service_manager->mqtt_class) {
        free(outside_service_manager->mqtt_class);
    }

    free(outside_service_manager);

    return 0;
}
#endif

int destroy_mqtt_share_element(mqtt_share_t *share) {
    pthread_mutex_destroy(&share->mqtt_message_list_mutex);
    pthread_cond_destroy(&share->mqtt_message_list_cond);
    destroy_message_list(&share->mqtt_message_list_head);

    return 0;
}

int destroy_mqtt_share(Mqtt_class_t *mqtt_class) {
    if (mqtt_class->recv_share) {
        free(mqtt_class->recv_share);
    }

    if (mqtt_class->send_share) {
        free(mqtt_class->send_share);
    }

    return 0;
}

int destroy_mqtt_class(esv_outside_service_manager_t *outside_service_manager) {
    if (outside_service_manager->mqtt_class) {
        if (outside_service_manager->mqtt_class->client_id) {
            free(outside_service_manager->mqtt_class->client_id);
        }
        free(outside_service_manager->mqtt_class);
    }

    return 0;
}

int destroy_mqtt_pthread(Mqtt_class_t *mqtt_class) {
    pthread_join(mqtt_class->recv_thread, NULL);
    pthread_join(mqtt_class->send_thread, NULL);

    return 0;
}

int init_share(mqtt_share_t *share) {
    pthread_mutex_init(&share->mqtt_message_list_mutex, NULL);
    pthread_cond_init(&share->mqtt_message_list_cond, NULL);
    share->mqtt_message_list_head = NULL;

    return 0;
}

int init_topic_to_composition_stamp(char *client_id) {
    time_t timestamp;
    get_time_stamp(&timestamp);
    composition_timestamp(&timestamp, client_id);

    return 0;
}

int init_mqtt_class(Mqtt_class_t *mqtt_class) {
    mqtt_class->recv_share = calloc(sizeof(mqtt_share_t), 1);
    mqtt_class->send_share = calloc(sizeof(mqtt_share_t), 1);
    // mqtt_class->recv_share = malloc(sizeof(mqtt_share_t));
    // mqtt_class->send_share = malloc(sizeof(mqtt_share_t));
    mqtt_class->client_id = malloc(sizeof(char) * 20);
    strcpy(mqtt_class->client_id, "Client");
    init_topic_to_composition_stamp(mqtt_class->client_id);
    init_share(mqtt_class->recv_share);
    init_share(mqtt_class->send_share);

    return 0;
}

#if 0
int init_manager_and_element(esv_outside_service_manager_t **outside_service_manager) {
    *outside_service_manager = calloc(1, sizeof(esv_outside_service_manager_t));
    if (*outside_service_manager == NULL) {
        perror("calloc error");
        return -1;
    }

    (*outside_service_manager)->mqtt_class = calloc(1, sizeof(Mqtt_class_t));
    init_mqtt_class((*outside_service_manager)->mqtt_class);

    return 0;
}
#endif

void *mqtt_service_thread_func(void *arg) {
    esv_outside_service_manager_t *outside_service_manager = (esv_outside_service_manager_t *) arg;

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync_createOptions createOpts = MQTTAsync_createOptions_initializer;
    createOpts.MQTTVersion = MQTTVERSION_5;

    int rc;
    int ch;

    nlog_info("recv_thread_func");
    if ((rc = MQTTAsync_createWithOptions(&outside_service_manager->mqtt_class->client, ADDRESS,
                                          outside_service_manager->mqtt_class->client_id, MQTTCLIENT_PERSISTENCE_NONE,
                                          NULL, &createOpts)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to create client,return code %d", rc);
    }
    //     if ((rc = MQTTAsync_create(&outside_service_manager->mqtt_class->client, ADDRESS,
    //                        outside_service_manager->mqtt_class->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL)) !=
    // MQTTASYNC_SUCCESS) {
    // nlog_warn("Failed to create client, return code %d\n", rc);
    // // goto exit;
    //     }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleanstart = 1;
    // conn_opts.cleansession = 1;
    //     conn_opts.onSuccess = onConnect;
    //     conn_opts.onFailure = onConnectFailure;
    conn_opts.automaticReconnect = 1;
    conn_opts.minRetryInterval = 2;
    conn_opts.maxRetryInterval = 5;
    conn_opts.context = outside_service_manager->mqtt_class->client;

    if ((rc = MQTTAsync_setCallbacks(outside_service_manager->mqtt_class->client, outside_service_manager, connlost,
                                     msgarrvd, NULL)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to set callbacks, return code %d\n", rc);
        // goto destroy_exit;
    }

    if ((rc = MQTTAsync_setConnected(outside_service_manager->mqtt_class->client, outside_service_manager->mqtt_class,
                                     onReconnected5)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to setConnected, return code %d\n", rc);
        // goto destroy_exit;
    }

    if ((rc = MQTTAsync_connect(outside_service_manager->mqtt_class->client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to start connect, return code %d\n", rc);
        // goto destroy_exit;
    }
    nlog_info("----");

    do {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');
    // pthread_join(plugin->send_thread, NULL);

    disc_opts.onSuccess = onDisconnect;
    disc_opts.onFailure = onDisconnectFailure;
    if ((rc = MQTTAsync_disconnect(outside_service_manager->mqtt_class->client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        nlog_warn("Failed to start disconnect, return code %d\n", rc);
    }

    // while (!disc_finished) {
    // usleep(10000);
    // }
    // destroy_exit:
    MQTTAsync_destroy(outside_service_manager->mqtt_class->client);

    pthread_exit(NULL);
}
