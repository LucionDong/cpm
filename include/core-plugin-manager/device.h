#ifndef ESV_DEVICE_H
#define ESV_DEVICE_H

#include <jansson.h>

#include "define.h"


typedef enum esv_thing_model_msg_type {
	ESV_TMM_JSON_OBJECT = 0,
} esv_thing_model_msg_type_e;

typedef struct {
    char *product_key;
    char *device_name;
    char *driver_name;
	char *thing_model_function_block_id;
	char *device_config;
} esv_device_info_t;

typedef struct {
	char                       *product_key;
	char                       *device_name;
	esv_thing_model_msg_type_e msg_type;
	void		*msg;
} esv_thing_model_msg_t;

int esv_device_info_cpy(esv_device_info_t *dest, const esv_device_info_t *src);
int esv_device_info_free(esv_device_info_t *device_info);
#endif
