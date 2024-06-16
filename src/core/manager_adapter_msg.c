#include "manager_internal.h"
#include "node_manager.h"
#include "utils/log.h"
#include "adapter/adapter_internal.h"
#include "manager_adapter_msg.h"
#include <stdlib.h>

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

int forward_thing_model_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg) {
	// 根据pk dn找到对应的node_name
	nlog_info("pk:%s dn:%s to find node_name", msg->product_key, msg->device_name);
	char *node_name = NULL;
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
