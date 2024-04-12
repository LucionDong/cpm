#ifndef _LAN_MQTT_SERVICE_H_
#define _LAN_MQTT_SERVICE_H_

#include <paho-mqtt/MQTTAsync.h>
#include "core/manager_internal.h"
#include "lan_mqtt_service_internal.h"

esv_lan_mqtt_service_t* esv_lan_mqtt_service_create();
int esv_lan_mqtt_srvice_set_manager(esv_lan_mqtt_service_t* service, neu_manager_t *manager);
int lan_mqtt_service_start(esv_lan_mqtt_service_t* service);
int lan_mqtt_service_stop(esv_lan_mqtt_service_t* service);
int lan_mqtt_service_publish(esv_lan_mqtt_service_t *service, char *topicName, char *msg);

#endif /* ifndef _LAN_MQTT_CLIENT_H_ */
