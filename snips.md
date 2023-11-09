```c
 case ESV_REQ_THING_PROPERTY_POST:
 case ESV_RESP_THING_PROPERTY_SET:
 case ESV_RESP_THING_PROPERTY_GET: {
     esv_reeqresp_thing_model_trans_data_t *cmd = (esv_reeqresp_thing_model_trans_data_t *) &header[1];
     nlog_info("driver: %s, product_key: %s, device_name: %s", cmd->driver, cmd->product_key, cmd->device_name);
     if (NULL != cmd->data_root) {
         char *data_root_str = json_dumps(cmd->data_root, 0);
         if (NULL != data_root_str) {
             nlog_info("thing model data root: %s", data_root_str);
             free(data_root_str);
         }
         json_decref(cmd->data_root);
     }
     break;
 }
 
 
   /* easeview start*/
  typedef struct {
      char                    driver[ESV_DRIVER_NAME_LEN];
      char                    product_key[ESV_PRODUCT_KEY_LEN];
      char                    device_name[ESV_DEVICE_NAME_LEN];
      json_t *                data_root;
  } esv_reeqresp_thing_model_trans_data_t;


	/* easeview */
	case ESV_REQ_THING_PROPERTY_POST:
	case ESV_RESP_THING_PROPERTY_SET:
	case ESV_RESP_THING_PROPERTY_GET: {
		esv_reeqresp_thing_model_trans_data_t *trans = (esv_reeqresp_thing_model_trans_data_t *)data;
		json_t *copied_data_root = NULL;
		if (NULL != trans->data_root) {
			copied_data_root = json_deep_copy(trans->data_root);	
		}

		data_size = sizeof(esv_reeqresp_thing_model_trans_data_t);

		nng_msg_alloc(&msg, sizeof(neu_reqresp_head_t) + data_size);
		body = nng_msg_body(msg);
		memcpy(body, header, sizeof(neu_reqresp_head_t));
		memcpy((uint8_t *) body + sizeof(neu_reqresp_head_t), data, data_size - sizeof(json_t *));
		neu_reqresp_head_t *header = (neu_reqresp_head_t *)body;
		esv_reeqresp_thing_model_trans_data_t *trans_data = (esv_reeqresp_thing_model_trans_data_t *) &header[1];
		trans_data->data_root = copied_data_root;
		return msg;
	}

    
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

	plugin->common.adapter_callbacks->esvdriver.func1(plugin->common.adapter);

    plog_notice(plugin, "start plugin `%s` success",
                neu_plugin_module.module_name);

    return rv;
error:
	if (NULL != data_root) {
		json_decref(data_root);
	}




```
