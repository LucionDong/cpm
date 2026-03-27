#include "manager_adapter_msg.h"

#include <jansson.h>
#include <stdlib.h>
#include <uuid.h>

#include "../adapter/storage.h"
// #include "adapter/adapter_internal.h"
#include "manager_internal.h"
#include "node_manager.h"
#include "parser_adapter_config.h"
#include "sql/sql_handle.h"
#include "utils/log.h"

/* static UT_array *esvdrivers = NULL; */

/* int forward_msg_to_esvdriver(neu_manager_t *manager, esv_between_adapter_driver_msg_t *msg) { */
/* 	if (esvdrivers == NULL) { */
/* 		esvdrivers = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVDRIVER); */
/* 	} */

/* 	if (esvdrivers == NULL) { */
/* 		nlog_warn("do not find driver!"); */
/* 		return -1; */
/* 	} */

/* 	/1* TODO:  <24-12-23, winston> */
/* 	 * 历遍 esvdrivers，调用 */
/* 	 * int rv = adapter->module->intf_funs->esvdriver.msg_to_driver(adapter->plugin, msg); */
/* 	 * *1/ */

/* end: */

/* 	return 0; */
/* } */
int forward_thing_model_matter_reload_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    char *recv_msg = strdup(msg->msg);
    // char *recv_msg = calloc(1, msg->msg_len);
    // strncpy(recv_msg, msg->msg, msg->msg_len);
    nlog_debug("recv_msg: %s", recv_msg);
    json_t *recv_config = json_loads(recv_msg, 0, NULL);

    json_t *params = json_object_get(recv_config, "params");
    const char *config_item_str = json_string_value(json_object_get(params, "configItem"));

    int config_result = 0;
    char *node_name = NULL, plugin_node_id[1024] = {0};
    if (config_item_str) {
        nlog_debug("msg.plugin_node_id: %d", msg->plugin_id);
        sprintf(plugin_node_id, "%d", msg->plugin_id);
    } else {
        const char *plugin_node = json_string_value(json_object_get(params, "pluginNodeId"));
        strcpy(plugin_node_id, plugin_node);
    }

    esv_persister_query_device_node_name_by_node_id(plugin_node_id, &node_name);
    nlog_info("plugin_id: %s,node_name: %s", plugin_node_id, node_name);
    if (NULL == node_name) {
        nlog_warn("do not find node of plugin_node_id: %s", plugin_node_id);
        config_result = 1;
    }
    neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, node_name);
    if (NULL == adapter) {
        nlog_warn("do not find adapter of node name: %s", node_name);
        return EXIT_FAILURE;
    }

    nlog_debug("config_item_str: %s", config_item_str);
    if (config_item_str &&
        (!strcmp(config_item_str, "deviceRegister") || !strcmp(config_item_str, "deviceUnregister") ||
         !strcmp(config_item_str, "deviceBatchUnregister") || !strcmp(config_item_str, "bridgeFactoryReset"))) {
        int ret = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, msg);
    } else if (NULL == config_item_str) {
        int ret = adapter->module->intf_funs->stop(adapter->plugin);
        nlog_debug("ret: %d", ret);
        if (ret != 0) {
            return 0;
        }

        if (esv_adapter_load_config(adapter->name, &adapter->setting) == 0) {
            nlog_debug("setting: %s", adapter->setting);
            if (adapter->module->intf_funs->setting(adapter->plugin, adapter->setting) == 0) {
                adapter->state = NEU_NODE_RUNNING_STATE_READY;
                config_result = 0;
                nlog_info("setting is OK");
            } else {
                free(adapter->setting);
                adapter->setting = NULL;
            }
        }
        adapter->module->intf_funs->start(adapter->plugin);
        nlog_debug("start over");
        send_config_plugin_results_for_matter(adapter, recv_config, config_result);
    }

    free(recv_msg);
    recv_msg = NULL;

    json_decref(recv_config);

    return 0;
}

int forward_thing_control_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    nlog_info("parser control msg");
    char *recv_msg = calloc(1, msg->msg_len);
    strncpy(recv_msg, msg->msg, msg->msg_len);
    nlog_debug("recv_msg: %s", recv_msg);
    json_t *recv_config = json_loads(recv_msg, 0, NULL);

    char *recv_config_str = json_dumps(recv_config, JSON_INDENT(2));
    nlog_debug("recv_config_str: %s", recv_config_str);
    free(recv_config_str);

    char *node_name = NULL;
    int config_result = 2;

    // 场景
    if (msg->method == ESV_TMM_MTD_WAN_SUBTHING_THING_PLUGIN_NODE_ACTION_PUSH) {
        json_t *msg_js = json_loads(msg->msg, 0, NULL);
        if (NULL == msg_js) {
            nlog_error("msg.msg is NULL");
            return -1;
        }

        json_t *params = json_object_get(msg_js, "params");
        const char *plugin_node_id = json_string_value(json_object_get(params, "pluginNodeId"));
        if (NULL == plugin_node_id) {
            nlog_error("plugin_node_id is NULL");
            return -1;
        }
        nlog_debug("plugin_node_id: %s", plugin_node_id);

        esv_persister_query_device_node_name_by_node_id(plugin_node_id, &node_name);
        nlog_debug("node_name:%s", node_name);
        if (NULL == node_name) {
            return EXIT_FAILURE;
        }
        neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, node_name);
        nlog_debug("neu_node_manager_find over");
        if (NULL == adapter) {
            nlog_warn("do not find adapter of node name: %s", node_name);
            return EXIT_FAILURE;
        }
        int ret = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, msg);
        nlog_debug("ret:%d", ret);
        if (ret != 0) {
            nlog_error("thing_model_msg_arrived error");
            return EXIT_FAILURE;
        }
        json_decref(msg_js);
    } else {
        int config_type = json_integer_value(json_object_get(recv_config, "configType"));
        const char *plugin_node_id =
            json_string_value(json_object_get(json_object_get(recv_config, "params"), "pluginNodeId"));
        nlog_debug("pluginNodeId: %s", plugin_node_id);

        esv_persister_query_device_node_name_by_node_id(plugin_node_id, &node_name);
        nlog_info("plugin_id: %s,node_name: %s", plugin_node_id, node_name);
        if (NULL == node_name) {
            nlog_warn("do not find node of plugin_node_id: %s", plugin_node_id);
            config_result = 1;
        }
        neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, node_name);
        if (NULL == adapter) {
            nlog_warn("do not find adapter of node name: %s", node_name);
            return EXIT_FAILURE;
        }
        adapter->module->intf_funs->stop(adapter->plugin);
        if (esv_adapter_load_config(adapter->name, &adapter->setting) == 0) {
            if (adapter->module->intf_funs->setting(adapter->plugin, adapter->setting) == 0) {
                adapter->state = NEU_NODE_RUNNING_STATE_READY;
                config_result = 0;
                nlog_info("setting is OK");
            } else {
                free(adapter->setting);
                adapter->setting = NULL;
            }
        }
        adapter->module->intf_funs->start(adapter->plugin);
        send_config_plugin_results(adapter, msg->product_key, msg->device_name, recv_config, config_result);
        goto clean_up;
    }

    neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, node_name);
    if (NULL == adapter) {
        nlog_warn("do not find adapter of node name: %s", node_name);
        return EXIT_FAILURE;
    }
    // 将信息发送至插件中,插件将transid存储起来
    // int rv = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, msg);
    // nlog_info("rv: %d", rv);
    // nlog_info("stop over");
    // if (rv != 0) {
    //     nlog_error("node_name: %s stop error", node_name);
    //     return EXIT_FAILURE;
    // }
    // nlog_info("++++++++++++start++++++++++");
    // if (rv != 0) {
    //     nlog_error("node_name: %s start error", node_name);
    //     return EXIT_FAILURE;
    // }
clean_up:
    if (recv_msg) {
        free(recv_msg);
        recv_msg = NULL;
    }
    if (node_name) {
        free(node_name);
        node_name = NULL;
    }

    return 0;
}

int forward_thing_model_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    // 根据pk dn找到对应的node_name
    nlog_info("pk:%s dn:%s to find node_name", msg->product_key, msg->device_name);
    UT_array *esv_self_device_drivers = NULL, *esv_node_name_array = NULL;
    char **temp = NULL;
    int rv = 0;
    esv_self_device_drivers = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVSELFDEVICEDRIVER);
    // char *node_name = NULL;
    // esv_persister_query_device_node_name(msg->product_key, msg->device_name, &node_name);

    esv_node_name_array = esv_persister_query_device_node_name_by_node_type();
    nlog_notice("esv_node_name_array: %d", utarray_len(esv_node_name_array));
    if (esv_self_device_drivers == NULL || esv_node_name_array == NULL) {
        nlog_warn("do not find node name in persister");
        return EXIT_FAILURE;
    }

    // if (NULL == node_name) {
    //     nlog_warn("do not find node of pk: %s dn: %s", msg->product_key, msg->device_name);
    //     utarray_foreach(esv_self_device_drivers, neu_adapter_t **, adapter) {
    //         nlog_debug("self adapter name: %s", (*adapter)->name);
    //
    //         char value[1024] = {0};
    //         select_plugin_node(manager->sql_handle, (*adapter)->name, value);
    //         int node_id = atoi(value);
    //         ((esv_thing_model_msg_t *) msg)->plugin_id = node_id;
    //         nlog_info("send adapter msg: %.*s", msg->msg_len, (char *) msg->msg);
    //         (*adapter)->module->intf_funs->esvdriver.thing_model_msg_arrived((*adapter)->plugin, msg);
    //     }
    //     return EXIT_SUCCESS;
    // }

    /* TODO:  <13-05-24, winston>
     * 根据plugin type分发数据
     * */

    // 根据node_name找到对应的adapter
    while ((temp = (char **) utarray_next(esv_node_name_array, temp))) {
        nlog_info("to find adapter of node_name:%s", *temp);
        // if (strcmp(*temp, "Matter") != 0) {
        //     continue;
        // }
        neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, *temp);
        if (NULL == adapter) {
            nlog_warn("do not find adapter of node name: %s", *temp);
            return EXIT_FAILURE;
        }

        char value[1024] = {0};
        select_plugin_node(manager->sql_handle, adapter->name, value);
        int node_id = atoi(value);
        nlog_info("node_name: %s, node_id: %d", adapter->name, node_id);
        ((esv_thing_model_msg_t *) msg)->plugin_id = node_id;

        nlog_info("to send ting model msg to esvdriver:%s", adapter->name);
        rv = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, msg);
    }
end:
    return rv;
}

/* TODO:  <16-06-24, yourname>
 * 需要释放esvdevicedrivernodes
 * */
static UT_array *esvdevicedrivernodes = NULL;
int forward_thing_model_msg_to_all_esvdevicedriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    if (esvdevicedrivernodes == NULL) {
        nlog_debug("to get adapter type %d", NEU_NA_TYPE_ESVDEVICEDRIVER);
        // esvdevicedrivernodes = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVDEVICEDRIVER);
        esvdevicedrivernodes = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVSELFDEVICEDRIVER);
    }

    if (esvdevicedrivernodes == NULL) {
        nlog_info("do not find esv device driver!");
        return -1;
    }

    utarray_foreach(esvdevicedrivernodes, neu_adapter_t **, adapter) {
        nlog_debug("send msg to device driver adapter %s", (*adapter)->name);

        char value[1024] = {0};
        select_plugin_node(manager->sql_handle, (*adapter)->name, value);
        int node_id = atoi(value);
        nlog_info("node_name: %s, node_id: %d", (*adapter)->name, node_id);
        ((esv_thing_model_msg_t *) msg)->plugin_id = node_id;

        nlog_info("send msg: %.*s", msg->msg_len, (char *) msg->msg);
        // nlog_info("send msg len: %d", msg->msg_len);
        (*adapter)->module->intf_funs->esvdriver.thing_model_msg_arrived((*adapter)->plugin, msg);
    }

    return 0;
}

/* TODO:  <16-06-24, yourname>
 * 需要释放esvappnodes
 * */
static UT_array *esvappnodes = NULL;
int forward_thing_model_msg_to_esvapps(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    if (esvappnodes == NULL) {
        nlog_debug("to get adapter type %d", NEU_NA_TYPE_ESVAPP);
        esvappnodes = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVAPP);
    }

    if (esvappnodes == NULL) {
        nlog_info("do not find esv app!");
        return -1;
    }

    utarray_foreach(esvappnodes, neu_adapter_t **, adapter) {
        nlog_debug("send msg to app adapter %s", (*adapter)->name);
        (*adapter)->module->intf_funs->esvdriver.thing_model_msg_arrived((*adapter)->plugin, msg);
    }

    return 0;
}

int forward_thing_model_msg_to_plugin_node(neu_manager_t *manager, const esv_thing_model_msg_t *msg,
                                           const char *plugin_node_id) {
    // 根据pluginNodeId找到对应的node_name
    nlog_info("plugin_node_id:%s to find node_name", plugin_node_id);
    char *node_name = NULL;
    esv_persister_query_device_node_name_by_node_id(plugin_node_id, &node_name);
    if (NULL == node_name) {
        nlog_warn("do not find node of pk: %s dn: %s", msg->product_key, msg->device_name);
        return EXIT_FAILURE;
    }

    // 根据node_name找到对应的adapter
    nlog_info("to find adapter of node_name:%s", node_name);
    neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, node_name);
    if (NULL == adapter) {
        nlog_warn("do not find adapter of node name: %s", node_name);
        return EXIT_FAILURE;
    }
    if (node_name) {
        free(node_name);
        node_name = NULL;
    }

    nlog_info("to send ting model msg to plugin node:%s", adapter->name);
    nlog_info("msg->msg: %.*s", msg->msg_len, (char *) msg->msg);
    nlog_info("msg->msg_len: %ld", strlen(msg->msg));
    int rv = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, msg);
end:
    return rv;
}
