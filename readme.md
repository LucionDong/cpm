- plugin 调用plugin->common.adapter_callbacks->esvdriver.msg_to_adapter往 outside_service_manager发送消息
- outside_service_manager 调用manager_adapter_msg.c中的forward_msg_to_esvdriver发送消息给 plugins

- 需要实现的 todo
1. src/core/outside_service_manager.c
1.1 esv_outside_service_manager_dispatch_msg 
2. src/adapter/driver/driver.c
2.1 esv_msg_to_adapter
3. src/core/manager_adapter_msg.c
3.1 forward_msg_to_esvdriver
