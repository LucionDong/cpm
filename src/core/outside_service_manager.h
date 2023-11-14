#ifndef _ESV_OUTSIDE_SERVICE_MANAGER_H_
#define _ESV_OUTSIDE_SERVICE_MANAGER_H_ 

#include "outside_service_manager_internal.h"
#include "manager_internal.h"

typedef enum esv_outside_service_reqresp_type {
	ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA = 0,
}esv_outside_service_reqresp_type_e;

static const char *esv_outside_service_reqresp_type_string_t[] = {
	[ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA] = "ESV_OUTSIDE_SERVICE_THING_MODEL_TRANS_DATA",
};

inline static const char *esv_outside_service_reqresp_type_string(esv_outside_service_reqresp_type_e type)
{
    return esv_outside_service_reqresp_type_string_t[type];
}

typedef struct esv_outside_service_reqresp_head {
    esv_outside_service_reqresp_type_e type;
    char               sender[64];
    char               receiver[64];
} esv_outside_service_reqresp_head_t;


esv_outside_service_manager_t *esv_outside_service_manager_create();
void esv_outside_service_manager_destory(esv_outside_service_manager_t *manager);
void esv_outside_service_set_neu_manager(esv_outside_service_manager_t *outside_service_manager, neu_manager_t *manager);
#endif /* ifndef _OUTSIDE_SERVICE_MANAGER_H_ */
