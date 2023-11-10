#ifndef ESV_DEVICE_H
#define ESV_DEVICE_H

#include "define.h"

/* typedef struct esv_driver_device esv_driver_device_t; */

/* typedef struct { */
/*     char product_key[ESV_PRODUCT_KEY_LEN]; */
/*     char device_name[ESV_DRIVER_NAME_LEN]; */
/* 	char thing_model_function_block_id[ESV_THING_MODEL_FUNC_BLOCK_ID_LEN]; */
/* 	char *device_config; */
/* } esv_device_info_t; */

typedef enum esv_thing_model_msg_type {
	ESV_TMM_JSON_OBJECT,
} esv_thing_model_msg_type_e;

typedef struct {
    char *product_key;
    char *device_name;
    char *driver_name;
	char *thing_model_function_block_id;
	char *device_config;
} esv_device_info_t;

int esv_device_info_cpy(esv_device_info_t *dest, const esv_device_info_t *src);
int esv_device_info_free(esv_device_info_t *device_info);
#endif
