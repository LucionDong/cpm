#include <stdio.h>
#include <stdlib.h>

#include "lnx-dmx_plugin.h"
#include "utils/log.h"
#include "plugin.h"
#include "errcodes.h"


const neu_plugin_module_t neu_plugin_module;

struct neu_plugin {
    neu_plugin_common_t common;
    char *              read_req_topic;
    char *              read_resp_topic;
};

static neu_plugin_t *mqtt_plugin_open(void)
{
    neu_plugin_t *plugin = (neu_plugin_t *) calloc(1, sizeof(neu_plugin_t));
    neu_plugin_common_init(&plugin->common);
    return plugin;
}

static int mqtt_plugin_close(neu_plugin_t *plugin)
{
    const char *name = neu_plugin_module.module_name;
    plog_notice(plugin, "success to free plugin:%s", name);

    free(plugin);
    return NEU_ERR_SUCCESS;
}

static int mqtt_plugin_init(neu_plugin_t *plugin, bool load)
{
    (void) load;
    plog_notice(plugin, "initialize plugin `%s` success",
                neu_plugin_module.module_name);
    return NEU_ERR_SUCCESS;
}

static int mqtt_plugin_uninit(neu_plugin_t *plugin)
{
    plog_notice(plugin, "uninitialize plugin `%s` success",
                neu_plugin_module.module_name);
    return NEU_ERR_SUCCESS;
}

static int mqtt_plugin_start(neu_plugin_t *plugin)
{
    int         rv          = 0;
    const char *plugin_name = neu_plugin_module.module_name;
    plog_notice(plugin, "start plugin `%s` success",
                neu_plugin_module.module_name);

    return rv;
}

static int mqtt_plugin_request(neu_plugin_t *plugin, neu_reqresp_head_t *head,
                               void *data)
{
    neu_err_code_e error = NEU_ERR_SUCCESS;
    plog_notice(plugin, "plugin `%s` request",
                neu_plugin_module.module_name);
	return error;
}

static int mqtt_plugin_stop(neu_plugin_t *plugin)
{
    plog_notice(plugin, "stop plugin `%s` success",
                neu_plugin_module.module_name);
    return NEU_ERR_SUCCESS;
}

static int mqtt_plugin_config(neu_plugin_t *plugin, const char *setting)
{
    int           rv          = 0;
    const char *  plugin_name = neu_plugin_module.module_name;
    bool          started     = false;

    plog_notice(plugin, "plugin `%s` config",
                neu_plugin_module.module_name);
	return rv;
}

static const neu_plugin_intf_funs_t plugin_intf_funs = {
    .open    = mqtt_plugin_open,
    .close   = mqtt_plugin_close,
    .init    = mqtt_plugin_init,
    .uninit  = mqtt_plugin_uninit,
    .start   = mqtt_plugin_start,
    .stop    = mqtt_plugin_stop,
    .setting = mqtt_plugin_config,
    .request = mqtt_plugin_request,
};

#define DESCRIPTION "Neuron northbound MQTT plugin bases on NanoSDK."
#define DESCRIPTION_ZH "基于 NanoSDK 的 Neuron 北向应用 MQTT 插件"

const neu_plugin_module_t neu_plugin_module = {
    .version         = NEURON_PLUGIN_VER_1_0,
    .schema          = "mqtt",
    .module_name     = "MQTT",
    .module_descr    = DESCRIPTION,
    .module_descr_zh = DESCRIPTION_ZH,
    .intf_funs       = &plugin_intf_funs,
    .kind            = NEU_PLUGIN_KIND_SYSTEM,
    .type            = NEU_NA_TYPE_APP,
    .display         = true,
    .single          = false,
};
