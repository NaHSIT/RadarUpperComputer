# 测风雷达数据格式文档

> 说明：本文保留当前 V1 帧和 `0x8100` 格式。V2 径向射线、标准风廓线、频谱、健康、告警和配置格式以 [ZYNQ7015_RADAR_DATA_AND_COMMUNICATION_SPEC.md](ZYNQ7015_RADAR_DATA_AND_COMMUNICATION_SPEC.md) 为准。

## 1. 文档信息

| 项目 | 内容 |
| --- | --- |
| 文档版本 | 1.1.0 |
| 适用对象 | 客户端协议开发、雷达端协议开发、联调测试 |
| 最大帧长度 | `4096` 字节 |
| 最小帧长度 | `14` 字节 |
| CRC 算法 | CRC-16/IBM 兼容实现 |
| 配套文档 | [DATA_INTERFACE.md](DATA_INTERFACE.md)、[COMM_PROTOCOL.md](COMM_PROTOCOL.md) |

## 2. 通用帧结构

```text
0                 2     4     6        10             ...      -4    -2
+-----------------+-----+-----+----------+----------------+-------+-----+
| Header AA55     | Len | Cmd | Sequence | Payload        | CRC16 | Tail |
+-----------------+-----+-----+----------+----------------+-------+-----+
     2 bytes       2 B   2 B      4 B         N bytes        2 B    2 B
```

| 偏移 | 字段 | 长度 | 字节序 | 说明 |
| --- | --- | --- | --- | --- |
| 0 | `header` | 2 | 大端 | 固定值 `0xAA55` |
| 2 | `length` | 2 | 大端 | 从 `command` 到 `crc16` 的总长度 |
| 4 | `command` | 2 | 大端 | 命令字 |
| 6 | `sequence` | 4 | 大端 | 请求/响应关联号 |
| 10 | `payload` | N | 业务定义 | 可以为空 |
| `10 + N` | `crc16` | 2 | 大端 | 对帧头到 payload 末尾做校验 |
| `12 + N` | `tail` | 2 | 大端 | 固定值 `0x55AA` |

## 3. 长度计算

### 3.1 `length` 字段定义

```text
length = 2(command) + 4(sequence) + payloadLength + 2(crc16)
```

### 3.2 总帧长定义

```text
totalFrameSize = 2(header) + 2(length field) + length + 2(tail)
               = 14 + payloadLength
```

注意：

1. `length` 不是 payload 长度。
2. 解析器不能重复把 `command` 和 `sequence` 计入总长度。

## 4. CRC 规则

| 项目 | 取值 |
| --- | --- |
| 算法 | CRC-16/IBM / Modbus 风格 |
| 初值 | `0xFFFF` |
| 多项式 | `0xA001` |
| 移位方向 | 右移 |
| 校验范围 | `header` 到 `payload` 最后一个字节 |
| 不参与校验 | `crc16` 字段本身与 `tail` |

伪代码：

```text
crc = 0xFFFF
for byte in frame[0 .. crcOffset - 1]:
    crc = crc XOR byte
    repeat 8 times:
        if (crc AND 1) == 1:
            crc = (crc >> 1) XOR 0xA001
        else:
            crc = crc >> 1
```

## 5. 基础命令数据格式

### 5.1 `0x0100` 查询设备信息

- 请求 payload：空
- 响应命令：`0x0000`
- 响应 payload：UTF-8 设备名称文本

示例请求：

```text
AA 55 00 08 01 00 00 00 00 01 27 DC 55 AA
```

### 5.2 `0x0200` 开始测量

- 请求 payload：空
- 响应命令：`0x0000`
- 响应 payload：空

### 5.3 `0x0201` 停止测量

- 请求 payload：空
- 响应命令：`0x0000`
- 响应 payload：空

## 6. `0x8100` 风场数据 payload 格式

### 6.1 概览

当前仿真端配置：

1. `gateCount = 30`
2. `rangeResolution = 10 m`
3. 固定总 payload 长度 `420` 字节
4. 总帧长 `434` 字节

### 6.2 字段表

| payload 偏移 | 字段 | 长度 | 字节序 | 类型 | 含义 |
| --- | --- | --- | --- | --- | --- |
| 0 | `timestampMs` | 8 | 小端 | `uint64` | 时间戳，毫秒 |
| 8 | `timeQuality` | 1 | 原样 | `uint8` | 时间质量标志 |
| 9 | `gateCount` | 2 | 大端 | `uint16` | 距离门数量 |
| 11 | `rangeResolutionM` | 4 | 小端 | `float32` | 距离分辨率 |
| 15 | `maxRangeM` | 4 | 小端 | `float32` | 最大测程 |
| 19 | `reserved` | 3 | 原样 | `byte[3]` | 保留字段 |
| 22 | `windSpeed[gateCount]` | `4 * count` | 小端 | `float32[]` | 各层风速 |
| `22 + 4*count` | `windDirection[gateCount]` | `4 * count` | 小端 | `float32[]` | 各层风向 |
| `22 + 8*count` | `verticalSpeed[gateCount]` | `4 * count` | 小端 | `float32[]` | 各层垂直速度 |
| `22 + 12*count` | `confidence[gateCount]` | `1 * count` | 原样 | `uint8[]` | 各层置信度 |
| `22 + 13*count` | `snrDb` | 4 | 小端 | `float32` | 信噪比 |
| `26 + 13*count` | `turbulence` | 4 | 小端 | `float32` | 湍流指标 |

### 6.3 固定 `gateCount = 30` 时的关键偏移

| 字段 | 偏移 |
| --- | --- |
| 第 0 层风速 | 22 |
| 第 0 层风向 | 142 |
| 第 0 层垂直速度 | 262 |
| 第 0 层置信度 | 382 |
| `snrDb` | 412 |
| `turbulence` | 416 |

### 6.4 当前仿真端业务含义

| 字段 | 当前仿真值说明 |
| --- | --- |
| `verticalSpeed` | 旧版 `0x8100` 兼容帧可为 `0`；三分量权威值由 `0x8105` 五波束反演产生 |
| `snrDb` | 当前固定约 `25.0` |
| `turbulence` | 当前固定约 `0.1` |
| `confidence` | 由仿真数据给出，范围 `0..100` |

## 7. 距离门换算规则

距离门索引从 `0` 开始。

```text
distanceM(i) = (i + 1) * rangeResolutionM
heightM(i)   = distanceM(i)
```

当前仿真中高度与距离直接等同，仅用于平视场景展示。真实雷达版本如果存在仰角、安装高度或地形修正，应新增独立字段，不应直接改写本字段语义。

`0x8105` 固定波束模式使用共同垂直高度网格：斜距按 `height / sin(elevation)` 换算。垂直波束仰角 `90°`，四个倾斜波束仰角 `75°`，因此斜距和垂直高度不得直接混用。

## 8. 字节序约定

当前协议存在混合字节序，必须显式处理：

| 范围 | 字节序 |
| --- | --- |
| 通用帧字段 | 大端 |
| `0x8100` 中的 `gateCount` | 大端 |
| `0x8100` 中的 `timestampMs` | 小端 |
| `0x8100` 中的浮点数 | 小端 |

实现要求：

1. 禁止直接把网络字节流映射为 C/C++ 结构体。
2. 必须使用显式读写函数，例如 `readU16BE`、`readU32BE`、`readU64LE`、`readFloatLE`。

## 9. 解析校验要求

1. 先验证帧头。
2. 再读取 `length` 计算完整帧长。
3. 验证帧尾。
4. 验证 CRC。
5. 根据命令字分发 payload 解析。
6. 对 `0x8100` 必须先读 `gateCount`，再计算理论 payload 最小长度。

## 10. 业务值校验建议

| 字段 | 建议校验 |
| --- | --- |
| 风速 | `0 .. 75 m/s` |
| 风向 | `0 <= direction < 360` |
| 置信度 | `0 .. 100` |
| `gateCount` | `1 .. 256` |
| 浮点数 | 不允许 `NaN`、`Infinity` |

## 11. 兼容性要求

1. 后续新增协议时优先新增命令字，不要破坏 `0x8100` 当前字段偏移。
2. 若必须升级 payload 结构，建议增加 `schemaVersion` 字段并发布新命令字。
3. 文本内容统一使用 UTF-8，避免依赖系统本地编码。
