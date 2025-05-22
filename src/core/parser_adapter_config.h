#include <jansson.h>

#include "metrics.h"
int send_config_plugin_results(neu_adapter_t *adapter, const char *parent_product_key, const char *parent_device_name,
                               json_t *recv_msg, int result);
int append_results(const char *plugin_node_id, int result, json_t **plugin_node_results);
