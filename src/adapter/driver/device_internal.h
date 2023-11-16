#ifndef _ESV_DEVICE_INTERNAL_H_
#define _ESV_DEVICE_INTERNAL_H_

#include <jansson.h>

#include "device.h"

typedef struct esv_thing_model_trans_data_inproc {
	esv_thing_model_msg_method_e method;
	char                    *driver;
	char                    *product_key;
	char                    *device_name;
	json_t                  *data_root;
} esv_thing_model_trans_data_inproc_t;

#endif /* ifndef _ESV_DEVICE_INTERNAL_H_ */
