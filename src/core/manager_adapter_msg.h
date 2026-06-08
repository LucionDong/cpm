#ifndef _MANAGER_ADAPTER_MSG_H_
#define _MANAGER_ADAPTER_MSG_H_

#include "manager_internal.h"
/* int forward_msg_to_esvdriver(neu_manager_t *manager, esv_between_adapter_driver_msg_t *msg); */
int forward_thing_model_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
int forward_thing_model_matter_reload_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
int forward_thing_model_msg_to_all_esvdevicedriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
int forward_thing_model_msg_to_esvapps(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
int forward_thing_model_msg_to_plugin_node(neu_manager_t *manager, const esv_thing_model_msg_t *msg,
                                           const char *plugin_node_id);
int forward_thing_control_msg_to_esvdriver(neu_manager_t *manager, const esv_thing_model_msg_t *msg);
/* --- 232/串口路由（并入自 core-plugin-manager-232）--- */
void parser_setting_to_uart_port(neu_adapter_t *adapter);
int  forward_msg_to_232esvdriver(neu_manager_t *manager, const uart_frame_t *msg);
int  forward_thing_model_msg_to_esvapp232s(neu_manager_t *manager, const esv_thing_model_msg_t *msg);

#endif /* ifndef _MANAGER_ADAPTER_MSG_H_ */
