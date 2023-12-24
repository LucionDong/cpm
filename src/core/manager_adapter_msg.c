#include "utils/log.h"
#include "adapter/adapter_internal.h"
#include "manager_adapter_msg.h"

UT_array *esvdrivers = NULL;
 
int forward_msg_to_esvdriver(neu_manager_t *manager, esv_between_adapter_driver_msg_t *msg) {
	if (esvdrivers == NULL) {
		esvdrivers = neu_node_manager_get_adapter(manager->node_manager, NEU_NA_TYPE_ESVDRIVER);	
	}

	if (esvdrivers == NULL) {
		nlog_warn("do not find driver!");
		return -1;
	}

	/* TODO:  <24-12-23, winston> 
	 * 历遍 esvdrivers，调用
	 * int rv = adapter->module->intf_funs->esvdriver.msg_to_driver(adapter->plugin, msg);
	 * */


end:
	
	return 0;
}
