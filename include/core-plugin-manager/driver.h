#ifndef _ESV_DRIVER_H_
#define _ESV_DRIVER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <nng/nng.h>

#include "adapter.h"

nng_msg *esv_nng_msg_gen(neu_reqresp_head_t *header, const void *data);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _ESV_DRIVER_H_ */
