#include "parser_adapter_config.h"

#include <jansson.h>
#include <stdlib.h>
#include <uuid.h>

#include "../adapter/adapter_internal.h"
#include "../connection/mqtt/lan_mqtt5_service.h"
#include "manager_internal.h"
#include "parser_adapter_config.h"
#include "utils/asprintf.h"

static char const hexdigits_lower[16] = "0123456789abcdef";

void uuid_to_string(const unsigned char *uuid, char *buf, char const *restrict fmt) {
    char *p = buf;

    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            *p++ = '-';
        }
        size_t tmp = uuid[i];
        *p++ = fmt[tmp >> 4];
        *p++ = fmt[tmp & 15];
    }
    *p = '\0';
}

void uuid_unparser(const unsigned char *uuid, char *buf) {
    uuid_to_string(uuid, buf, hexdigits_lower);
}

// 发送成功信息
int append_results(const char *plugin_node_id, int result, json_t **plugin_node_results) {
    json_t *plugin_node_result_element = json_object();

    json_object_set_new(plugin_node_result_element, "result", json_integer(result));
    json_object_set_new(plugin_node_result_element, "pluginNodeId", json_string(plugin_node_id));
    json_array_append(*plugin_node_results, plugin_node_result_element);

    return 0;
}
int send_config_plugin_results_for_matter(neu_adapter_t *adapter, json_t *recv_msg, int result) {
    unsigned char uuid[16];
    char *uuid_str = calloc(1, sizeof(char) * 37);
    uuid_generate(uuid);
    uuid_unparser(uuid, uuid_str);

    char *trans_id = strdup(json_string_value(json_object_get(recv_msg, "transId")));
    char *plugin_node_id =
        strdup(json_string_value(json_object_get(json_object_get(recv_msg, "params"), "pluginNodeId")));
    int config_type = json_integer_value(json_object_get(json_object_get(recv_msg, "params"), "pluginNodeId"));

    json_t *send_json = json_object(), *data = json_object(), *plugin_node_result = json_object();

    json_object_set_new(send_json, "id", json_string(uuid_str));
    json_object_set_new(send_json, "transId", json_string(trans_id));
    json_object_set_new(send_json, "code", json_string("200"));
    json_object_set_new(send_json, "method", json_string("thing.pluginNode.config.pushReply"));

    json_object_set_new(data, "configType", json_integer(config_type));

    json_object_set_new(plugin_node_result, "result", json_integer(result));
    json_object_set_new(plugin_node_result, "pluginNodeId", json_string(plugin_node_id));
    json_object_set_new(data, "configResult", plugin_node_result);

    json_object_set_new(send_json, "data", data);

    char *data_root_str = json_dumps(send_json, 0);
    // esv_thing_model_msg_t thing_model_msg = {.method = ESV_TMM_MTD_APP_THING_PLUGIN_NODE_CONFIG_RELOAD_PUSH_REPLY,
    //                                          .product_key = "",
    //                                          .device_name = "",
    //                                          .msg_type = ESV_TMM_JSON_STRING_PTR,
    //                                          .msg = data_root_str};
    // if (ESV_TMM_JSON_STRING_PTR == thing_model_msg.msg_type) {
    char *topic_formate = "app/thing/pluginNode/config/pushReply";
    lan_mqtt5_service_publish(adapter->lan_mqtt5_service, topic_formate, data_root_str);
    // }

    free(uuid_str);
    free(data_root_str);
    json_decref(data);
    return 0;
}

int send_config_plugin_results(neu_adapter_t *adapter, const char *parent_product_key, const char *parent_device_name,
                               json_t *recv_msg, int result) {
    unsigned char uuid[16];
    char *uuid_str = calloc(1, sizeof(char) * 37);
    uuid_generate(uuid);
    uuid_unparser(uuid, uuid_str);

    char *trans_id = strdup(json_string_value(json_object_get(recv_msg, "transId")));
    // char *device_iot_id =
    //     strdup(json_string_value(json_object_get(json_object_get(recv_msg, "params"), "deviceIotId")));
    char *plugin_node_id =
        strdup(json_string_value(json_object_get(json_object_get(recv_msg, "params"), "pluginNodeId")));
    int config_type = json_integer_value(json_object_get(json_object_get(recv_msg, "params"), "pluginNodeId"));

    json_t *send_json = json_object(), *data = json_object(), *plugin_node_result = json_object();

    json_object_set_new(send_json, "id", json_string(uuid_str));
    json_object_set_new(send_json, "transId", json_string(trans_id));
    // json_object_set_new(send_json, "version", json_string("1.0"));
    json_object_set_new(send_json, "code", json_string("200"));
    json_object_set_new(send_json, "method", json_string("thing.pluginNode.config.pushReply"));

    json_object_set_new(data, "configType", json_integer(config_type));
    json_object_set_new(data, "deviceIotId", json_string(""));

    json_object_set_new(plugin_node_result, "result", json_integer(result));
    json_object_set_new(plugin_node_result, "pluginNodeId", json_string(plugin_node_id));
    json_object_set_new(data, "configResult", plugin_node_result);

    json_object_set_new(send_json, "data", data);

    char *product_key = strdup(parent_product_key);
    char *device_name = strdup(parent_device_name);
    char *data_root_str = json_dumps(send_json, 0);
    esv_thing_model_msg_t thing_model_msg = {.method = ESV_TMM_MTD_WAN_SUBTHING_THING_PLUGIN_NODE_CONFIG_PUSH_REPLY,
                                             .product_key = product_key,
                                             .device_name = device_name,
                                             .msg_type = ESV_TMM_JSON_STRING_PTR,
                                             .msg = data_root_str};
    if (ESV_TMM_JSON_STRING_PTR == thing_model_msg.msg_type) {
        char *topic_formate = "wan/thing/pluginNode/config/pushReply";
        // neu_asprintf(&topic, topic_formate, thing_model_msg.product_key, thing_model_msg.device_name);
        lan_mqtt5_service_publish(adapter->lan_mqtt5_service, topic_formate, thing_model_msg.msg);
    }

    free(product_key);
    free(device_name);
    free(uuid_str);
    free(data_root_str);
    json_decref(data);
    return 0;
}
