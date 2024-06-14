#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils/log.h"
#include "utils/mqtt.h"
#include "utils/asprintf.h"
#include "utils/time.h"
#include "core/manager_adapter_msg.h"
#include "lan_mqtt5_service.h"
#include "lan_mqtt5_service_internal.h"

#define ADDRESS     "tcp://127.0.0.1:1883"
#define CLIENTID    "cpm-lan-mqtt"
#define QOS         1
#define TIMEOUT     10000L

// topic
// wan
#define TOPIC_WILDCARD_WAN_THINGSUB_PROPERTY_SET "wan/+/+/thing/sub/+/+/thing/service/property/set"
#define TOPIC_WILDCARD_WAN_THINGSUB_PROPERTY_GET "wan/+/+/thing/sub/+/+/thing/service/property/get"
// lan
#define TOPIC_WILDCARD_LAN_THINGSUB_EVENT_PROPERTY_POST "lan/thing/sub/+/+/thing/event/property/post"
#define TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_SET "lan/thing/sub/+/+/thing/service/property/set"
#define TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_SETREPLY "lan/thing/sub/+/+/thing/service/property/setReply"
#define TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_GET "lan/thing/sub/+/+/thing/service/property/get"
#define TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_GETREPLY "lan/thing/sub/+/+/thing/service/property/getReply"

/* int finished = 0; */

struct esv_lan_mqtt5_service_s {
	neu_manager_t *manager;
	MQTTAsync client;
};


void error_reporting_onSubscribeSuccess5(void* context, MQTTAsync_successData5* response)
{
	MQTTAsync c = (MQTTAsync)context;
	int rc;
	int i = 0;

	if (response->alt.sub.reasonCodeCount == 1)
		nlog_debug("reason code %d", response->reasonCode);
	else if (response->alt.sub.reasonCodeCount > 1)
	{
		for (i = 0; i < response->alt.sub.reasonCodeCount; ++i)
		{	int code = response->alt.sub.reasonCodes[i];
			nlog_debug("Subscribe reason code %d -> %s", code , MQTTReasonCode_toString(code));
		}
	}
}

void connected5(void *context, char *cause)
{
	nlog_notice("Connected5");
	esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context;
	nlog_info("contex=%p service=%p, service client=%p", context, service, service->client);
	int topic_count = 7;
	MQTTAsync client = service->client;
	/* MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer; */
	MQTTSubscribe_options subscribe_options = MQTTSubscribe_options_initializer;
	subscribe_options.noLocal = 1;
	MQTTAsync_callOptions copts = MQTTAsync_callOptions_initializer;
	copts.subscribeOptions = subscribe_options;
	copts.subscribeOptionsCount = topic_count;
	copts.onSuccess5 = error_reporting_onSubscribeSuccess5;
	copts.context = context;
	int rc;
	char *topics_to_subscribe[] = {
		TOPIC_WILDCARD_WAN_THINGSUB_PROPERTY_SET, 
		TOPIC_WILDCARD_WAN_THINGSUB_PROPERTY_GET, 
		TOPIC_WILDCARD_LAN_THINGSUB_EVENT_PROPERTY_POST,
		TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_SET,
		TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_SETREPLY,
		TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_GET,
		TOPIC_WILDCARD_LAN_THINGSUB_SERVICE_PROPERTY_GETREPLY
	}; 
	const int qos[] = {QOS, QOS, QOS, QOS, QOS, QOS, QOS};
	MQTTSubscribe_options subopts[] = {subscribe_options, subscribe_options, subscribe_options, subscribe_options, subscribe_options, subscribe_options, subscribe_options};
	copts.subscribeOptionsList = subopts;
	if ((rc = MQTTAsync_subscribeMany(client, topic_count, topics_to_subscribe, qos, &copts)) != MQTTASYNC_SUCCESS)
	{
		nlog_warn("Failed to start subscribe, return code %d", rc);
	}

	nlog_debug("subscribe5 success");
	/* if ((rc = MQTTAsync_subscribe(client, TOPIC_WILDCARD_LAN_SUBTHING_PROPERTY_SET, QOS, &opts)) != MQTTASYNC_SUCCESS) */
	/* { */
	/* 	nlog_warn("Failed to start subscribe, return code %d", rc); */
	/* } */

	/* nlog_notice("Reconnecting"); */
	/* conn_opts.keepAliveInterval = 20; */
	/* conn_opts.cleansession = 1; */
	/* if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) */
	/* { */
	/* 	nlog_error("Failed to start connect, return code %d", rc); */
 		/* finished = 1; */
	/* } */
}

void connlost5(void *context, char *cause)
{
	esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context;
	MQTTAsync client = service->client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	nlog_warn("Connection lost");
	if (cause) {
		nlog_warn("     cause: %s", cause);
	}

	/* nlog_notice("Reconnecting"); */
	/* conn_opts.keepAliveInterval = 20; */
	/* conn_opts.cleansession = 1; */
	/* if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) */
	/* { */
	/* 	nlog_error("Failed to start connect, return code %d", rc); */
 		/* finished = 1; */
	/* } */
}

void onDisconnectFailure5(void* context, MQTTAsync_failureData* response)
{
	nlog_notice("Disconnect failed");
	/* finished = 1; */
}

void onDisconnect5(void* context, MQTTAsync_successData* response)
{
	nlog_notice("Successful disconnection");
	/* finished = 1; */
}

void onSendFailure5(void* context, MQTTAsync_failureData* response)
{
	/* MQTTAsync client = (MQTTAsync)context; */
	/* MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer; */
	/* int rc; */

	nlog_warn("Message send failed token %d error code %d", response->token, response->code);
	/* opts.onSuccess = onDisconnect; */
	/* opts.onFailure = onDisconnectFailure; */
	/* opts.context = client; */
	/* if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) */
	/* { */
	/* 	nlog_error("Failed to start disconnect, return code %d", rc); */
	/* 	exit(EXIT_FAILURE); */
	/* } */
}

void onSend5(void* context, MQTTAsync_successData* response)
{
	/* MQTTAsync client = (MQTTAsync)context; */
	/* MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer; */
	/* int rc; */

	nlog_notice("Message with token value %d delivery confirmed", response->token);
	/* opts.onSuccess = onDisconnect; */
	/* opts.onFailure = onDisconnectFailure; */
	/* opts.context = client; */
	/* if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) */
	/* { */
	/* 	nlog_notice("Failed to start disconnect, return code %d", rc); */
	/* 	exit(EXIT_FAILURE); */
	/* } */
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	nlog_notice("Connect failed, rc %d", response ? response->code : 0);
	/* finished = 1; */
}


void onConnect5(void* context, MQTTAsync_successData* response)
{
	/* MQTTAsync client = (MQTTAsync)context; */
	/* MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer; */
	/* MQTTAsync_message pubmsg = MQTTAsync_message_initializer; */
	/* int rc; */
	nlog_notice("Successful connection");
	esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context;
	nlog_info("contex=%p service=%p, service client=%p", context, service, service->client);

	/* opts.onSuccess = onSend; */
	/* opts.onFailure = onSendFailure; */
	/* opts.context = client; */
	/* pubmsg.payload = PAYLOAD; */
	/* pubmsg.payloadlen = (int)strlen(PAYLOAD); */
	/* pubmsg.qos = QOS; */
	/* pubmsg.retained = 0; */
	/* if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) */
	/* { */
	/* 	nlog_notice("Failed to start sendMessage, return code %d", rc); */
	/* 	exit(EXIT_FAILURE); */
	/* } */
}

int messageArrived5(void* context, char* topicName, int topicLen, MQTTAsync_message* m)
{
	/* not expecting any messages */
	nlog_info("msg arrived topic: %s", topicName);
	if (topic_matches_wildcard(topicName, TOPIC_WILDCARD_WAN_THINGSUB_PROPERTY_SET)) {
		char *pk;
		char *dn;
		get_pk_dn_from_thingsub_topic(topicName, 5, &pk, &dn);		
		nlog_debug("pk:%s dn:%s", pk, dn);
		esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context;
		esv_thing_model_msg_t *thing_model_msg = calloc(1, sizeof(esv_thing_model_msg_t));
		thing_model_msg->method = ESV_TMM_MTD_WAN_SUBTHING_THING_SERVICE_PROPERTY_SET;
		thing_model_msg->product_key = pk;
		thing_model_msg->device_name = dn;
		thing_model_msg->msg_type = ESV_TMM_JSON_STRING_PTR;
		thing_model_msg->msg = m->payload;
		forward_thing_model_msg_to_esvdriver(service->manager, thing_model_msg);
end1:		
		free(pk);
		free(dn);
		free(thing_model_msg);
		MQTTAsync_freeMessage(&m);
		MQTTAsync_free(topicName);

	} else if (topic_matches_wildcard(topicName, TOPIC_WILDCARD_WAN_THINGSUB_PROPERTY_GET)) {
		char *pk;
		char *dn;
		get_pk_dn_from_thingsub_topic(topicName, 5, &pk, &dn);		
		nlog_debug("pk:%s dn:%s", pk, dn);
		esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context;
		esv_thing_model_msg_t *thing_model_msg = calloc(1, sizeof(esv_thing_model_msg_t));
		thing_model_msg->method = ESV_TMM_MTD_WAN_SUBTHING_THING_SERVICE_PROPERTY_GET;
		thing_model_msg->product_key = pk;
		thing_model_msg->device_name = dn;
		thing_model_msg->msg_type = ESV_TMM_JSON_STRING_PTR;
		thing_model_msg->msg = m->payload;
		forward_thing_model_msg_to_esvdriver(service->manager, thing_model_msg);
end2:		
		free(pk);
		free(dn);
		free(thing_model_msg);
		MQTTAsync_freeMessage(&m);
		MQTTAsync_free(topicName);

	}
	/* if (topic_matches_wildcard(topicName, TOPIC_WILDCARD_WAN_SUBTHING_PROPERTY_SET)) { */
	/* 	/1* nlog_info("              msg: \n%.*s", m->payloadlen, (char*)m->payload); *1/ */
	/* 	char *topic2 = strdup(topicName); */
	/* 	char *token; */
	/* 	char *pk; */
	/* 	char *dn; */
	/* 	int token_count = 0; */
	/* 	token = strtok(topic2, "/"); */
	/* 	while (token != NULL) { */
	/* 		if (token_count == 4) { */
	/* 			pk = strdup(token); */
	/* 		} else if (token_count == 5) { */
	/* 			dn = strdup(token); */
	/* 		} */
	/* 		if (token_count == 5) { */
	/* 			break; */
	/* 		} */
	/* 		token_count++; */
	/* 		token = strtok(NULL, "/"); */		
	/* 	} */
	/* 	nlog_debug("pk:%s dn:%s", pk, dn); */
	/* 	esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context; */
	/* 	esv_thing_model_msg_t *thing_model_msg = calloc(1, sizeof(esv_thing_model_msg_t)); */
	/* 	thing_model_msg->method = ESV_TMM_MTD_WAN_SUBTHING_THING_SERVICE_PROPERTY_SET; */
	/* 	thing_model_msg->product_key = pk; */
	/* 	thing_model_msg->device_name = dn; */
	/* 	thing_model_msg->msg_type = ESV_TMM_JSON_STRING_PTR; */
	/* 	thing_model_msg->msg = m->payload; */
	/* 	forward_thing_model_msg_to_esvdriver(service->manager, thing_model_msg); */
/* end1: */		
	/* 	free(topic2); */
	/* 	free(pk); */
	/* 	free(dn); */
	/* 	free(thing_model_msg); */
	/* 	MQTTAsync_freeMessage(&m); */
	/* 	MQTTAsync_free(topicName); */
	/* } */
	/* else if (topic_matches_wildcard(topicName, TOPIC_WILDCARD_WAN_SUBTHING_PROPERTY_GET)) { */
	/* 	/1* nlog_info("              msg: \n%.*s", m->payloadlen, (char*)m->payload); *1/ */
	/* 	char *topic2 = strdup(topicName); */
	/* 	char *token; */
	/* 	char *pk; */
	/* 	char *dn; */
	/* 	int token_count = 0; */
	/* 	token = strtok(topic2, "/"); */
	/* 	while (token != NULL) { */
	/* 		if (token_count == 4) { */
	/* 			pk = strdup(token); */
	/* 		} else if (token_count == 5) { */
	/* 			dn = strdup(token); */
	/* 		} */
	/* 		if (token_count == 5) { */
	/* 			break; */
	/* 		} */
	/* 		token_count++; */
	/* 		token = strtok(NULL, "/"); */		
	/* 	} */
	/* 	nlog_debug("pk:%s dn:%s", pk, dn); */
	/* 	esv_lan_mqtt5_service_t *service = (esv_lan_mqtt5_service_t *)context; */
	/* 	esv_thing_model_msg_t *thing_model_msg = calloc(1, sizeof(esv_thing_model_msg_t)); */
	/* 	thing_model_msg->method = ESV_TMM_MTD_WAN_SUBTHING_THING_SERVICE_PROPERTY_GET; */
	/* 	thing_model_msg->product_key = pk; */
	/* 	thing_model_msg->device_name = dn; */
	/* 	thing_model_msg->msg_type = ESV_TMM_JSON_STRING_PTR; */
	/* 	thing_model_msg->msg = m->payload; */
	/* 	forward_thing_model_msg_to_esvdriver(service->manager, thing_model_msg); */
/* end2: */		
	/* 	free(topic2); */
	/* 	free(pk); */
	/* 	free(dn); */
	/* 	free(thing_model_msg); */
	/* 	MQTTAsync_freeMessage(&m); */
	/* 	MQTTAsync_free(topicName); */
	/* } */
	return 1;
}

int lan_mqtt5_service_publish(esv_lan_mqtt5_service_t *service, char *topicName, char *msg) {
	nlog_debug("publish topic:%s", topicName);
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	pubmsg.payload = msg;
    pubmsg.payloadlen = (int)strlen(msg);
    pubmsg.qos = 1;
    pubmsg.retained = 0;

	nlog_debug("+++mqtt async send");
	if ((rc = MQTTAsync_sendMessage(service->client, topicName, &pubmsg, NULL)) != MQTTASYNC_SUCCESS)
	{
		nlog_warn("Failed to start sendMessage, return code %d", rc);
		return EXIT_FAILURE;
	}
	nlog_debug("---mqtt async send");
	return EXIT_SUCCESS;
}

esv_lan_mqtt5_service_t* esv_lan_mqtt5_service_create() {
	esv_lan_mqtt5_service_t* service = calloc(1, sizeof(esv_lan_mqtt5_service_t));
	return service;
}

int esv_lan_mqtt5_srvice_set_manager(esv_lan_mqtt5_service_t* service, neu_manager_t *manager) {
	service->manager = manager;
	return EXIT_SUCCESS;
}

int lan_mqtt5_service_start(esv_lan_mqtt5_service_t* service) {
	int rc;
	MQTTAsync lan_mqtt5_client;
	MQTTAsync_connectOptions lan_mqtt5_conn_opts = MQTTAsync_connectOptions_initializer5;
	MQTTAsync_createOptions createOpts = MQTTAsync_createOptions_initializer;
	createOpts.MQTTVersion = MQTTVERSION_5;
	/* if ((rc = MQTTAsync_create(&lan_mqtt5_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS) */
	char *client_id = NULL;
	long long ct = current_timestamp();
	nlog_debug("current timestamp = %lld", ct);
	int ret = neu_asprintf(&client_id, "%s|timestamp=%lld|", CLIENTID, ct);
	if ((rc = MQTTAsync_createWithOptions(&lan_mqtt5_client, ADDRESS, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL, &createOpts)) != MQTTASYNC_SUCCESS)
	{
		nlog_error("Failed to create client object, return code %d", rc);
		return rc;
	}
	service->client = lan_mqtt5_client;

	nlog_info("lan_mqtt5_client=%p, service=%p, client=%p", lan_mqtt5_client, service, service->client);


	nlog_debug("lan_mqtt5_conn_opts.MQTTVersion = %d", lan_mqtt5_conn_opts.MQTTVersion);
	/* lan_mqtt5_conn_opts.MQTTVersion = MQTTVERSION_5; */
	lan_mqtt5_conn_opts.keepAliveInterval = 20;
	lan_mqtt5_conn_opts.cleanstart = 1;
	lan_mqtt5_conn_opts.automaticReconnect = 1;
	lan_mqtt5_conn_opts.onSuccess = onConnect5;
	lan_mqtt5_conn_opts.onFailure = onConnectFailure;
	lan_mqtt5_conn_opts.context = service;

	if ((rc = MQTTAsync_setCallbacks(lan_mqtt5_client, lan_mqtt5_conn_opts.context, connlost5, messageArrived5, NULL)) != MQTTASYNC_SUCCESS)
	{
		nlog_error("Failed to set callback, return code %d", rc);
		return rc;
	}

	if ((rc = MQTTAsync_setConnected(lan_mqtt5_client, lan_mqtt5_conn_opts.context, connected5)) != MQTTASYNC_SUCCESS)
	{
		nlog_error("Failed to set  connected, return code %d", rc);
		return rc;
	}

	if ((rc = MQTTAsync_connect(lan_mqtt5_client, &lan_mqtt5_conn_opts)) != MQTTASYNC_SUCCESS)
	{
		nlog_warn("Failed to start connect, return code %d", rc);
		/* exit(EXIT_FAILURE); */
	}
	nlog_info("lan_mqtt5_client=%p, service=%p, client=%p, contex=%p", lan_mqtt5_client, service, service->client, lan_mqtt5_conn_opts.context);

	return rc;
}

int lan_mqtt5_service_stop(esv_lan_mqtt5_service_t* service) {
	MQTTAsync lan_mqtt5_client = service->client;
	MQTTAsync_destroy(&lan_mqtt5_client);
	return EXIT_SUCCESS;
}


