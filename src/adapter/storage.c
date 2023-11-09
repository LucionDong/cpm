#include "utils/log.h"

#include "adapter_internal.h"
#include "driver/driver_internal.h"
#include "storage.h"
#include <stdint.h>
#include <stdlib.h>

void adapter_storage_state(const char *node, neu_node_running_state_e state)
{
    neu_persister_update_node_state(node, state);
}

void adapter_storage_setting(const char *node, const char *setting)
{
    neu_persister_store_node_setting(node, setting);
}

int adapter_load_setting(const char *node, char **setting)
{
    int rv = neu_persister_load_node_setting(node, (const char **) setting);
    if (0 != rv) {
        nlog_warn("load %s setting fail", node);
        return -1;
    }

    return 0;
}

int adapter_load_device(neu_adapter_driver_t *driver) {
	UT_array *persist_device_infos = NULL;
	neu_adapter_t *adapter     = (neu_adapter_t *) driver;	

	int rv = esv_persister_load_devices(adapter->name, &persist_device_infos);
	if (0 != rv) {
		nlog_warn("load %s device fail", adapter->name);
		return rv;
	}
	
	/* nlog_info("load %s %d device success", adapter->name, utarray_len(persist_device_infos)); */
	uint16_t device_cnt = utarray_len(persist_device_infos);
	esv_device_info_t *device_infos = (esv_device_info_t *)calloc(device_cnt, sizeof(esv_device_info_t));

	int index = 0;
	utarray_foreach(persist_device_infos, esv_persist_device_info_t *, p) {
		device_infos[index].product_key = strdup(p->product_key);
		device_infos[index].device_name = strdup(p->device_name);
		device_infos[index].driver_name = strdup(p->driver_name);
		device_infos[index].thing_model_function_block_id = strdup(p->thing_model_function_block_id);
		device_infos[index].device_config = strdup(p->device_config);
		index++;
	}

	esv_adapter_driver_load_devices(driver, device_cnt, device_infos);
	
end:
	for (int i = 0; i < device_cnt; i++) {
		esv_device_info_free(&device_infos[i]);
	}
	free(device_infos);
	utarray_free(persist_device_infos);
	return rv;
}
