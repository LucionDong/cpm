# core-plugin-manager

运行在本地网关上的**插件化设备接入与消息转发框架**。订阅云端/本地 MQTT5
Broker 上的物模型消息（属性设置/读取、事件上报、场景联动、配置下发等），
按设备归属路由到对应驱动插件，并将设备状态反向上报，实现云平台与本地设备的
双向通信。

## 核心能力

- **单进程双族管理**：统一管理 KNX/以太网设备（ESVDEVICEDRIVER 族）与
  RS485/RS232 串口设备（232 族），无需多进程经 Broker 桥接。
- **物模型消息路由**：SET/GET/POST/UPDATE 等方法按 product_key / device_name
  精确投递到归属节点，App 层广播，支持跨族 handoff。
- **动态插件加载**：从数据库登记表加载驱动 `.so`（dlopen），按类型白名单
  校验，运行期实例化为适配器节点。
- **串口子系统**：通过 MCU 串口（RS232 帧协议）与下游 485/232 设备通信，
  含帧解析、CRC 校验、网关串口配置下发。
- **MQTT5 接入**：订阅 wan/lan 多组主题通配符，物模型消息进出统一收口。

## 技术栈

C (C11) · Paho MQTT5 · SQLite · zlog · jansson · CMake
交叉编译目标：armv5l（ELF 32-bit ARM, EABI5）

## 架构概览

```
        云端 MQTT5 Broker
              │ wan/... · lan/...
              ▼
        core-plugin-manager（单进程）
         ├─ KNX 族插件（knxAdapter / device_map / rule / weather …）
         └─ 232 族插件（customuart / moorgen / carrier …）→ MCU 串口 → 485/232 总线
```

## 构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

产物为 `build/core-plugin-manager`（交叉编译目标产物名为 `esvcpm-<version>`），
驱动插件 `.so` 放入运行目录下的 `plugins/`，框架启动时按数据库登记自动加载。
