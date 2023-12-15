# Easeview局域网网关发现

*   局域网广播端口：`5354`
*   广播内容：`EaseMore Gateway Search`
*   广播内容类型：`string utf-8`
*   广播回应：
    ```json
    {
        "deviceId":"02:8B:B6:35:6F:41",
        "deviceIp":"10.1.1.121"
    }
    ```

# Easeview基于Mqtt的数据传输协议

## 基于MQTT 的KNX消息格式

### MQTT连接参数

*   broker TCP 端口号：1883
*   username: 3part_username
*   password: 3part_password
*   SET_CLEAN_SESSION: true
*   MQTT Version：3.1.1

### topic

*   说明
    *   appId：客户端appId
    *   serviceId：客户端发起连接的serviceId

#### publish Topic

*   /sys/${appId}/${serviceId}/thing/event/property/post
*   ~~/sys/${appId}/${serviceId}/thing/event/property/post_reply~~
*   /sys/${appId}/${serviceId}/thing/event/property/post_ack

#### subscribe Topic

*   说明
    *   +：表示通配一个层级，例如 a/+ 匹配 a/x， a/y
*   /sys/+/+/thing/event/property/post
*   ~~/sys/+/+/thing/event/property/post_reply~~
*   /sys/+/+/thing/event/property/post_ack

#### Payload

*   请求Topic携带的payload：
    *   发布主题：/sys/${appId}/${serviceId}/thing/event/property/post
    *   订阅主题：/sys/+/+/thing/event/property/post
    ```json
    {
        "msgId":"12ff70c6-2e85-11ed-a261-0242ac120002",
        "timestamp": "1524448722000", // post请求时间戳
        "appId":"plugin",
        "serviceId":"daikin-hum",
        "ackType":"ack",
        "data": { //KNX数据
            "knx":[
                {
                    "sourceAddress": "", //第三方客户端忽略此字段
                    "destinationAddress": "",
                    "commandType": "", // Write或Read或Response
                    "valueType": "",
                    "value": ""
                },
                {
                    "sourceAddress": "",//第三方客户端忽略此字段
                    "destinationAddress": "",
                    "commandType": "", // Write或Read或Response
                    "valueType": "",
                    "value": ""            
                }        
            ]
            
        },
        "method": "thing.event.property.post", // 大金插件请求
        "version": "1.0.0" // 请求json版本号
    }
    ```
*   应答topic携带的payload：
    *   每个客户端收到post消息后，需要向发送的source_appId，source_serviceId返回post_ack,msgIg需要和post消息保持一致
    *   发布主题：/sys/${source_appId}/${source_serviceId}/thing/event/property/post_ack
    *   订阅主题：/sys/+/+/thing/event/property/post_ack
    ```json
    {
       "code": 200, // 请求结果状态码
       "msgId":"12ff70c6-2e85-11ed-a261-0242ac120002", //需要与post的msgId保持一直
       "timestamp": "1524448722000", // ack发送时间戳
       "appId":"gatewaybrokercore", // 发送ack的client的appId
       "serviceId":"emlocalmqtt", // 发送ack的client的serviceId
       "message": "success", // 应答描述
       "method": "thing.event.property.post_ack", 
       "version": "1.0.0" //应答json版本号
    }
    ```
    
    ### 消息示例
    *   请求Topic携带的payload：
    *   发布主题：/sys/${appId}/${serviceId}/thing/event/property/post
    *   订阅主题：/sys/+/+/thing/event/property/post
    ```json
    {
        "msgId":"12ff70c6-2e85-11ed-a261-0242ac120002",
        "timestamp": "1524448722000", // post请求时间戳
        "appId":"huali",
        "serviceId":"huali-mqtt",
        "ackType":"ack",
        "data": { //KNX数据
            "knx":[
                {
                    "sourceAddress": "", //第三方客户端忽略此字段
                    "destinationAddress": "1/1/1",//knx 组地址 1/1/1
                    "commandType": "Read", // Write或Read或Response
                    "valueType": "", // 数据类型1Bit，1Byte，2Byte
                    "value": "" // 发送的值
                },
                {
                    "sourceAddress": "",//第三方客户端忽略此字段
                    "destinationAddress": "2/1/3",
                    "commandType": "Write", // Write或Read或Response
                    "valueType": "1Bit",
                    "value": "1"            
                }        
            ]
            
        },
        "method": "thing.event.property.post", // 大金插件请求
        "version": "1.0.0" // 请求json版本号
    }
    ```

