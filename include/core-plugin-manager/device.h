#ifndef _ESV_DEVICE_H_
#define _ESV_DEVICE_H_

#include <jansson.h>

#include "define.h"

typedef enum esv_thing_model_msg_method {
	ESV_TMM_MTD_THING_SERVICE_PROPERTY_SET = 0,
	ESV_TMM_MTD_THING_SERVICE_PROPERTY_SET_REPLY,
	ESV_TMM_MTD_THING_SERVICE_PROPERTY_GET,
	ESV_TMM_MTD_THING_SERVICE_PROPERTY_GET_REPLY,
	ESV_TMM_MTD_THING_EVENT_PROPERTY_POST,
} esv_thing_model_msg_method_e;

typedef enum esv_thing_model_msg_type {
	ESV_TMM_JSON_OBJECT_PTR = 0,
} esv_thing_model_msg_type_e;

typedef struct {
    char *product_key;
    char *device_name;
    char *driver_name;
	char *thing_model_function_block_id;
	char *device_config;
} esv_device_info_t;


typedef struct esv_thing_model_trans_data_ipc {
	esv_thing_model_msg_method_e method;
	char                    product_key[ESV_PRODUCT_KEY_LEN];
	char                    device_name[ESV_DEVICE_NAME_LEN];
	char					json_bytes[];
} esv_thing_model_trans_data_ipc_t;


typedef struct {
	esv_thing_model_msg_method_e method;
	char                       *product_key;
	char                       *device_name;
	esv_thing_model_msg_type_e msg_type;
	void		*msg;
} esv_thing_model_msg_t;

int esv_device_info_cpy(esv_device_info_t *dest, const esv_device_info_t *src);
int esv_device_info_free(esv_device_info_t *device_info);
#endif
