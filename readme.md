- plugin 调用plugin->common.adapter_callbacks->esvdriver.msg_to_adapter往 outside_service_manager发送消息
- outside_service_manager 调用manager_adapter_msg.c中的forward_msg_to_esvdriver发送消息给 plugins
- node中plugin_name对应plugin so中module_name

- 需要实现的 todo
1. src/core/outside_service_manager.c
1.1 esv_outside_service_manager_dispatch_msg 
2. src/adapter/driver/driver.c
2.1 esv_msg_to_adapter
3. src/core/manager_adapter_msg.c
3.1 forward_msg_to_esvdriver

### cpm启动流程
1. main()->

### cpm默认配置
- 配置目录：./config/
- zlog.conf: ./config/zlog.conf
- plugin: ./plugins/
- default_plugins.json中填写的是so的全名，cpm启动时会加载这个文件里面的so，根据so中neu_plugin_module中的module_name存储到hash表中，node实例化的时候会根据数据库plugin_node表中的plugin_name字段中的值与hash表中的key匹配从而已对应的value作为so去加载

### useage
#### topic
##### 设备主动上报属性：
  - topic: `lan/subthing/${subthingProductKey}/${subthingDeviceName}/thing/event/property/post`
     - 方向：plugin->plugin_manager
     - 说明：网关内部插件发送的指令
##### 查询设备属性
  - 请求
    -  topic: `wan/${productKey}/${deviceName}/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/get`
        - 方向：plugin_manager->plugin
        - 说明：云端发送的指令
    - topic: `lan/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/get`
       - 方向：plugin_manager->plugin
       - 说明：网关内部发送的指令
  - 应答
    -  topic: `wan/${productKey}/${deviceName}/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/getReply`
        - 方向：plugin->plugin_manager
        - 说明：返回云端的指令
    - topic: `lan/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/getReply`
       - 方向：plugin->plugin_manager
       - 说明：返回网关内部的指令
##### 设置设备属性
  - 请求
    -  topic: `wan/${productKey}/${deviceName}/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/set`
        - 方向：plugin_manager->plugin
        - 说明：云端发送的指令
    - topic: `lan/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/set`
       - 方向：plugin_manager->plugin
       - 说明：网关内部发送的指令
  - 应答
    -  topic: `wan/${productKey}/${deviceName}/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/setReply`
        - 方向：plugin->plugin_manager
        - 说明：返回云端的指令
    - topic: `lan/subthing/${subthingProductKey}/${subthingDeviceName}/thing/service/property/setReply`
       - 方向：plugin->plugin_manager
       - 说明：返回网关内部的指令