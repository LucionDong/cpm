#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

#include "utils/uthash.h"

#include "define.h"
#include "device.h"


/* typedef struct { */
/*     char product_key[ESV_PRODUCT_KEY_LEN]; */
/*     char device_name[ESV_DRIVER_NAME_LEN]; */
/* } tkey_t; */

/* struct esv_driver_device_elem { */
/* 	esv_device_property_t device_property; */

/* 	tkey_t key; */
/* 	UT_hash_table hh; */
/* }; */

/* struct esv_driver_device { */
/* 	nng_mtx *mtx; */

/* 	struct esv_driver_device_elem *devices_table; */
/* }; */

/* inline static tkey_t to_key(const char *product_key, const char *device_name) { */
/* 	tkey_t key = { 0 }; */

/* 	strcpy(key.product_key, product_key); */
/* 	strcpy(key.device_name, device_name); */

/* 	return key; */
/* } */

/* esv_driver_device_t *esv_driver_device_new() { */
/* 	esv_driver_device_t *device = calloc(1, sizeof(esv_driver_device_t)); */

/* 	nng_mtx_alloc(&device->mtx); */

/* 	return device; */
/* } */

int esv_device_info_cpy(esv_device_info_t *dest, const esv_device_info_t * src) {
	dest->product_key = strdup(src->product_key);
	dest->device_name = strdup(src->device_name);
	dest->driver_name = strdup(src->driver_name);
	dest->thing_model_function_block_id = strdup(src->thing_model_function_block_id);
	dest->device_config = strdup(src->device_config);
	return 0;
}

int esv_device_info_free(esv_device_info_t *device_info) {
	free(device_info->product_key);
	free(device_info->device_name);
	free(device_info->driver_name);
	free(device_info->thing_model_function_block_id);
	free(device_info->device_config);
	return 0;
}
