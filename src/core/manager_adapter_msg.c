#include <jansson.h>
#include <stdlib.h>

#include "adapter/adapter_internal.h"
#include "config/easeview_user_config.h"
#include "manager_adapter_msg.h"
#include "manager_internal.h"
#include "node_manager.h"
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

void parser_setting_to_uart_port(neu_adapter_t *adapter) {
    json_error_t error;
    json_t *root = json_loads(adapter->setting, 0, &error);
    adapter->uart_port = strdup(json_string_value(json_object_get(root, "uartPort")));
}

int forward_thing_model_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    // 根据pk dn找到对应的node_name
    nlog_info("pk:%s dn:%s to find node_name", msg->product_key, msg->device_name);
    char *node_name = NULL;
    /* TODO:  <24-06-24, winston>
     * 需要优化，不要每次都从数据库读取，从缓存读取
     * */
    esv_persister_query_device_node_name(msg->product_key, msg->device_name, &node_name);
    if (NULL == node_name) {
        nlog_warn("do not find node of pk: %s dn: %s", msg->product_key, msg->device_name);
        return EXIT_FAILURE;
    }

    /* TODO:  <13-05-24, winston>
     * 根据plugin type分发数据
     * */

    // 根据node_name找到对应的adapter
    nlog_info("to find adapter of node_name:%s", node_name);
    neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, node_name);
    if (NULL == adapter) {
        nlog_warn("do not find adapter of node name: %s", node_name);
        return EXIT_FAILURE;
    }
    nlog_info("to send ting model msg to esvdriver:%s", adapter->name);
    int rv = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, msg);
end:
    return rv;
}

static UT_array *esv_app_driver_232_config_nodes = NULL;
static UT_array *esv_app_driver_232_nodes = NULL;
int forward_msg_to_232esvdriver(neu_manager_t *manager, const esv_frame232_msg_t *msg) {
    //根据msg中的类型发送到对应的232插件中
    // msg类型设计主要应为从数据库中读取到的串口号或者可以单独标识插件
    // utarray_new(esv_app_driver_232_config_nodes, &ut_str_icd);
    // load_uart_config_from_db(esv_app_driver_232_config_nodes);

    if (esv_app_driver_232_nodes == NULL) {
        nlog_debug("to get adapter type %d", NEU_NA_TYPE_ESVAPPDRIVER232);
        esv_app_driver_232_nodes = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVAPPDRIVER232);
    }

    if (esv_app_driver_232_nodes == NULL) {
        nlog_info("do not find esv appdriver232");
        return -1;
    }

    utarray_foreach(esv_app_driver_232_nodes, neu_adapter_t **, adapter) {
        //校验每个adapter的串口号是否一致
        //一致后向该adapter发送信息
        //该回调函数由插件实现
        parser_setting_to_uart_port(*adapter);
        if (!strcmp((*adapter)->uart_port, msg->serial_port_num)) {
            (*adapter)->module->intf_funs->esvdriver.app_driver_232_msg_arrived((*adapter)->plugin, msg);
        }
    }
    return 0;
}

/* TODO:  <16-06-24, yourname>
 * 需要释放esvappnodes
 * */
static UT_array *esvapp232nodes = NULL;
int forward_thing_model_msg_to_esvapp232s(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
    if (esvapp232nodes == NULL) {
        nlog_debug("to get adapter type %d", NEU_NA_TYPE_ESVAPP232);
        esvapp232nodes = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVAPP232);
    }

    if (esvapp232nodes == NULL) {
        nlog_info("do not find esv app232!");
        return -1;
    }

    utarray_foreach(esvapp232nodes, neu_adapter_t **, adapter) {
        nlog_debug("send msg to app adapter %s", (*adapter)->name);
        (*adapter)->module->intf_funs->esvdriver.thing_model_msg_arrived((*adapter)->plugin, msg);
    }

    return 0;
}
