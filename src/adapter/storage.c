#include "utils/log.h"

#include "adapter_internal.h"
#include "driver/driver_internal.h"
#include "storage.h"
#include <stdint.h>
#include <stdlib.h>

// easeview
int esv_adapter_load_config(const char *node, char **config)
{
    int rv = esv_persister_load_node_config(node, (const char **) config);
    if (0 != rv) {
        nlog_warn("load %s setting fail", node);
        return -1;
    }

    return 0;
}
// easeview end


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

	esv_adapter_driver_load_devices(driver, persist_device_infos);
	
end:
	utarray_free(persist_device_infos);
	return rv;
}
