#ifndef _MANAGER_ADAPTER_MSG_H_
#define _MANAGER_ADAPTER_MSG_H_

#include "manager_internal.h"
/* int forward_msg_to_esvdriver(neu_manager_t *manager, esv_between_adapter_driver_msg_t *msg); */
int forward_thing_model_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
int forward_thing_model_msg_to_esvapp232s(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
int forward_msg_to_232esvdriver(neu_manager_t *manager, const esv_frame232_msg_t *msg);
void parser_setting_to_uart_port(neu_adapter_t *adapter);

#endif /* ifndef _MANAGER_ADAPTER_MSG_H_ */
