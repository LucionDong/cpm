#include "utils/log.h"
#include "persist/persist.h"
#include "adapter/adapter_internal.h"
#include "manager_thing_model.h"
#include <stdlib.h>

 
int forward_thing_model_msg(neu_manager_t *manager, esv_thing_model_trans_data_ipc_t *msg) {
	char *driver_name = NULL;
	esv_persister_query_device_driver(msg->product_key, msg->device_name, &driver_name);
	if (driver_name == NULL) {
		nlog_warn("pk:%s dn:%s do not find driver!", msg->product_key, msg->device_name);
		return -1;
	}

	neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, driver_name);
	if (adapter == NULL) {
		nlog_warn("do not find %s driver!", driver_name);
		return -2;
	}

	if (adapter->module->type != NEU_NA_TYPE_ESVDRIVER) {
		nlog_warn("driver:%s is not NEU_NA_TYPE_ESVDRIVER", driver_name);
		return -3;
	}

	if (adapter->module->intf_funs->esvdriver.thing_model_msg_arrived == NULL) {
		nlog_warn("driver:%s thing_model_msg_arrived is NULL", driver_name);
		return -4;
	}

	json_error_t error;
	json_t *msg_root = json_loads(msg->json_bytes, 0, &error);
	if (!msg_root) {
		nlog_warn("decode msg json string is null!");
		return -5;
	}

	esv_thing_model_msg_t thing_modle_msg = {
		.product_key = strdup(msg->product_key),
		.device_name = strdup(msg->device_name),
		.msg_type = ESV_TMM_JSON_OBJECT_PTR,
		.msg = msg_root,
	};

	int rv = adapter->module->intf_funs->esvdriver.thing_model_msg_arrived(adapter->plugin, &thing_modle_msg);

end:
	free(thing_modle_msg.product_key);
	free(thing_modle_msg.device_name);
	json_decref(msg_root);
	
	return 0;
}
