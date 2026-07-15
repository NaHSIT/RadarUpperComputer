# 测风雷达技术文档目录

## 1. 当前已补齐文档

| 文档 | 作用 |
| --- | --- |
| [DATA_INTERFACE.md](DATA_INTERFACE.md) | 定义客户端与雷达端的数据接口边界、命令和会话规则 |
| [DATA_FORMAT.md](DATA_FORMAT.md) | 定义二进制帧格式与 `0x8100` payload 字段格式 |
| [COMM_PROTOCOL.md](COMM_PROTOCOL.md) | 单独说明通信协议、连接流程、状态机、联调要求 |
| [DEPLOYMENT_AND_NETWORK.md](DEPLOYMENT_AND_NETWORK.md) | 说明客户端、雷达端、浏览器维护端的部署与组网建议 |
| [PYART_INTEGRATION.md](PYART_INTEGRATION.md) | 定义 Py-ART 风场反演、质量控制、C++ 适配和标准产品 |
| [ZYNQ7015_RADAR_DATA_AND_COMMUNICATION_SPEC.md](ZYNQ7015_RADAR_DATA_AND_COMMUNICATION_SPEC.md) | 定义 Zynq-7015 硬件数据链路、板内记录、TCP V1/V2、浏览器接口及雷达产品格式 |
| [ZYNQ_UPPER_COMPUTER_DATA_INTERFACE_GUIDE.md](ZYNQ_UPPER_COMPUTER_DATA_INTERFACE_GUIDE.md) | 面向 Zynq 开发的当前上位机联调实施说明、C 编码方法和黄金测试帧 |

## 2. 建议下一步继续补的文档

下面这些文档在真正进入多人开发、联调和交付阶段会很有价值：

1. 《错误码与告警码文档》：明确 `0x0001` 以及后续业务错误的编码规则。
2. 《配置项字典文档》：定义设备参数、算法参数、网络参数、校准参数。
3. 《数据库设计文档》：如果雷达端或客户端需要历史存储，应定义表结构与索引。
4. 《用户与权限设计文档》：客户端与浏览器维护端的角色权限矩阵。
5. 《模块详细设计文档》：按总览页、风场页、波束页、健康页、设置页逐页细化逻辑。
6. 《日志与诊断规范》：统一日志级别、字段、滚动策略、问题定位流程。
7. 《联调测试用例文档》：覆盖连接、测量、掉线、CRC 错误、超时、异常数据等场景。
8. 《发布部署手册》：面向现场安装、升级、回滚、备份恢复。

## 3. 推荐文档体系

如果我们要把这一套做成正式项目文档，建议最终形成下面四层：

1. 总体设计：系统架构、模块边界、部署方案。
2. 接口协议：TCP 协议、HTTP API、WebSocket 推送、数据格式。
3. 详细设计：页面逻辑、状态机、配置项、权限、日志。
4. 测试交付：联调手册、测试用例、发布手册、运维手册。
