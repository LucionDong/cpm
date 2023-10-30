#include "utils/log.h"

#include "adapter_internal.h"
#include "driver/driver_internal.h"
#include "storage.h"

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
