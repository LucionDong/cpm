#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #include "lnx-dmx_plugin.h" */
#include "utils/log.h"
#include "plugin.h"
#include "errcodes.h"


const neu_plugin_module_t neu_plugin_module;

struct neu_plugin {
    neu_plugin_common_t common;
    char *              read_req_topic;
    char *              read_resp_topic;
};

static neu_plugin_t *dmx_plugin_open(void)
{
    neu_plugin_t *plugin = (neu_plugin_t *) calloc(1, sizeof(neu_plugin_t));
    neu_plugin_common_init(&plugin->common);
    return plugin;
}

static int dmx_plugin_close(neu_plugin_t *plugin)
{
    const char *name = neu_plugin_module.module_name;
    plog_notice(plugin, "success to free plugin:%s", name);

    free(plugin);
    return NEU_ERR_SUCCESS;
}

static int dmx_plugin_init(neu_plugin_t *plugin, bool load)
{
    (void) load;
    plog_notice(plugin, "initialize plugin `%s` success",
                neu_plugin_module.module_name);
    return NEU_ERR_SUCCESS;
}

static int dmx_plugin_uninit(neu_plugin_t *plugin)
{
    plog_notice(plugin, "uninitialize plugin `%s` success",
                neu_plugin_module.module_name);
    return NEU_ERR_SUCCESS;
}

static int dmx_plugin_start(neu_plugin_t *plugin)
{
    int         rv          = 0;
    const char *plugin_name = neu_plugin_module.module_name;

	/* plugin->common.adapter_callbacks->driver.sync_driver_devices(plugin->common.adapter, NULL); */

	neu_reqresp_head_t header = { 0 };
    header.type               = ESV_REQ_THING_PROPERTY_POST;
	esv_reeqresp_thing_model_trans_data_t *data = calloc(1, sizeof(esv_reeqresp_thing_model_trans_data_t));
	
	
    plog_notice(plugin, "to copy data");
	strcpy(data->driver, plugin_name);
	strcpy(data->product_key, "pk_^*^%$dfa");
	strcpy(data->device_name, "dn_^*^%$dfa");

	json_t *data_root = json_object();
	json_t *id_str = json_string("thing_model_id_str");
	json_object_set_new(data_root, "id", id_str);

	data->data_root = data_root;

    plog_notice(plugin, "to do plugin op");
	if (0 != neu_plugin_op(plugin, header, data)) {
		plog_error(plugin, "neu_plugin_op(ESV_REQ_THING_PROPERTY_POST) fail");
    }

	if (NULL != data_root) {
		json_decref(data_root);
	}
    plog_notice(plugin, "start plugin `%s` success",
                neu_plugin_module.module_name);

    return rv;
error:
	if (NULL != data_root) {
		json_decref(data_root);
	}
	return -1;
}

static int dmx_plugin_request(neu_plugin_t *plugin, neu_reqresp_head_t *head,
                               void *data)
{
    neu_err_code_e error = NEU_ERR_SUCCESS;
    plog_notice(plugin, "plugin `%s` request",
                neu_plugin_module.module_name);
	return error;
}

static int dmx_plugin_stop(neu_plugin_t *plugin)
{
    plog_notice(plugin, "stop plugin `%s` success",
                neu_plugin_module.module_name);
    return NEU_ERR_SUCCESS;
}

static int dmx_plugin_config(neu_plugin_t *plugin, const char *setting)
{
    int           rv          = 0;
    const char *  plugin_name = neu_plugin_module.module_name;
    bool          started     = false;

    plog_notice(plugin, "plugin `%s` config",
                neu_plugin_module.module_name);
	return rv;
}

static const neu_plugin_intf_funs_t plugin_intf_funs = {
    .open    = dmx_plugin_open,
    .close   = dmx_plugin_close,
    .init    = dmx_plugin_init,
    .uninit  = dmx_plugin_uninit,
    .start   = dmx_plugin_start,
    .stop    = dmx_plugin_stop,
    .setting = dmx_plugin_config,
    .request = dmx_plugin_request,
};

#define DESCRIPTION "LNX-900M LED Driver plugin bases on NanoSDK."
#define DESCRIPTION_ZH "LNX-900M LED控制器，适用于LNX-900M系列子型号，软件版本为V2.01及以上兼容软件版本"

const neu_plugin_module_t neu_plugin_module = {
    .version         = NEURON_PLUGIN_VER_1_0,
    .schema          = "lnx900-dmx",
    .module_name     = "LNX900-DMX",
    .module_descr    = DESCRIPTION,
    .module_descr_zh = DESCRIPTION_ZH,
    .intf_funs       = &plugin_intf_funs,
    .kind            = NEU_PLUGIN_KIND_SYSTEM,
    .type            = NEU_NA_TYPE_DRIVER,
    .display         = true,
    .single          = false,
};
