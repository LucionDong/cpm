[ ] paho mqtt v5 订阅主题设置的qos=1，但订阅成功后返回的reasonCode=2
[x] paho mqtt v5 connectOptions 已经用MQTTAsync_connectOptions_initializer5初始化，为何还需要手动设置lan_mqtt5_conn_opts.MQTTVersion = MQTTVERSION_5;否则连接时会报版本不一致的错误
[ ] 需要替换为通过node id查询device list
