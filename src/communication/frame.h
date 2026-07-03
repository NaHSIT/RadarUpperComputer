#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <QByteArray>

// ==================== 帧结构常量 ====================
const uint16_t FRAME_HEADER = 0xAA55;      // 帧头
const uint16_t FRAME_TAIL = 0x55AA;        // 帧尾

const int FRAME_HEADER_SIZE = 2;           // 帧头 2字节
const int FRAME_LENGTH_SIZE = 2;           // 长度 2字节
const int FRAME_CMD_SIZE = 2;              // 命令 2字节
const int FRAME_SEQ_SIZE = 4;              // 序号 4字节
const int FRAME_CRC_SIZE = 2;              // CRC 2字节
const int FRAME_TAIL_SIZE = 2;             // 帧尾 2字节

// 最小帧长度 = 帧头(2) + 长度(2) + 命令(2) + 序号(4) + CRC(2) + 帧尾(2) = 14
const int FRAME_MIN_SIZE = FRAME_HEADER_SIZE + FRAME_LENGTH_SIZE +
                          FRAME_CMD_SIZE + FRAME_SEQ_SIZE +
                          FRAME_CRC_SIZE + FRAME_TAIL_SIZE;

// ==================== 命令码定义 ====================

// 命令码分类
enum class CommandCategory : uint16_t {
    SYSTEM    = 0x0000,    // 系统命令 (0x0000-0x00FF)
    CONFIG    = 0x0100,    // 配置命令 (0x0100-0x01FF)
    CONTROL   = 0x0200,    // 控制命令 (0x0200-0x02FF)
    DATA      = 0x0300,    // 数据命令 (0x0300-0x03FF)
};

// 具体命令码
enum class CommandCode : uint16_t {
    // ========== 系统命令 ==========
    CMD_HEARTBEAT          = 0x0001,    // 心跳包
    CMD_STATUS_QUERY       = 0x0002,    // 状态查询
    CMD_STATUS_REPORT      = 0x0003,    // 状态上报
    CMD_SYSTEM_RESET       = 0x0004,    // 系统复位
    CMD_SHUTDOWN           = 0x0005,    // 关机
    CMD_VERSION_QUERY      = 0x0006,    // 版本查询
    CMD_VERSION_REPORT     = 0x0007,    // 版本上报

    // ========== 配置命令 ==========
    CMD_READ_CONFIG        = 0x0100,    // 读取配置
    CMD_WRITE_CONFIG       = 0x0101,    // 写入配置
    CMD_DEFAULT_CONFIG     = 0x0102,    // 恢复默认
    CMD_SAVE_CONFIG        = 0x0103,    // 保存到Flash

    // ========== 控制命令 ==========
    CMD_START_MEASURE      = 0x0200,    // 启动测量
    CMD_STOP_MEASURE       = 0x0201,    // 停止测量
    CMD_SWITCH_BEAM        = 0x0202,    // 波束切换
    CMD_SET_PRF            = 0x0203,    // 设置PRF
    CMD_SET_PULSE_WIDTH    = 0x0204,    // 设置脉冲宽度
    CMD_SET_GAIN           = 0x0205,    // 设置增益
    CMD_SET_RANGE_GATE     = 0x0206,    // 设置距离门
    CMD_CALIBRATE          = 0x0207,    // 触发校准

    // ========== 数据命令 ==========
    CMD_ECHO_DATA          = 0x0300,    // 回波数据
    CMD_WIND_DATA          = 0x0301,    // 风场数据
    CMD_SPECTRUM_DATA      = 0x0302,    // 谱数据
    CMD_IQ_DATA            = 0x0303,    // IQ数据
    CMD_DATA_REQUEST       = 0x0304,    // 数据请求

    // ========== 响应命令 ==========
    CMD_ACK                = 0x0080,    // 确认应答
    CMD_NACK               = 0x0081,    // 否定应答
    CMD_ERROR              = 0x0082,    // 错误应答
};

// ==================== 帧结构 ====================

#pragma pack(push, 1)

// 帧头部
struct FrameHeader {
    uint16_t frameHeader;   // 帧头 0xAA55
    uint16_t length;        // 帧长度 (不含帧头帧尾)
    uint16_t command;       // 命令码
    uint32_t sequence;      // 序列号
};

// 帧尾部
struct FrameTail {
    uint16_t crc;           // CRC16校验
    uint16_t frameTail;     // 帧尾 0x55AA
};

#pragma pack(pop)

// ==================== 系统状态 ====================

// 系统状态码
enum class SystemState : uint8_t {
    IDLE        = 0x00,     // 空闲
    INIT        = 0x01,     // 初始化
    RUNNING     = 0x02,     // 运行中
    MEASURING   = 0x03,     // 测量中
    ERROR       = 0xFF,     // 错误
};

// 系统状态结构
struct SystemStatus {
    SystemState state;       // 系统状态
    uint8_t beamIndex;       // 当前波束
    uint8_t errorCount;      // 错误计数
    float temperature;       // 温度 (℃)
    float voltage;           // 电压 (V)
    uint32_t uptime;         // 运行时间 (秒)
};

// ==================== 风场数据 ====================

// 风场数据结构
struct WindFieldData {
    uint64_t timestamp;           // 时间戳 (微秒)
    uint8_t beamIndex;            // 波束索引 (0-4)
    uint16_t gateCount;           // 距离门数
    float gateResolution;         // 距离分辨率 (m)
    float maxRange;               // 最大距离 (m)
    float windSpeed[30];          // 风速 (m/s) 最多30层
    float windDirection[30];      // 风向 (度)
    float verticalSpeed[30];      // 垂直速度 (m/s)
    uint8_t confidence[30];       // 置信度 (0-100)
    float snr;                    // 平均信噪比 (dB)
    float turbulence;             // 湍流强度
};

#endif // FRAME_H
