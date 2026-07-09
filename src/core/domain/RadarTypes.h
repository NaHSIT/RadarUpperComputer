#ifndef RADARTYPES_H
#define RADARTYPES_H

#include <cstdint>
#include <QString>
#include <QDateTime>
#include <QVector>

/**
 * @brief 雷达通用类型定义
 */

// 连接状态
enum class ConnectionState {
    Offline,        // 离线
    Connecting,     // 连接中
    Online,         // 在线
    DataTimeout,    // 数据超时
    ProtocolError   // 协议错误
};

// 工作模式
enum class WorkMode {
    Standby,        // 待机
    Initializing,   // 初始化
    Running,        // 运行
    Measuring,      // 测量
    Debug,          // 调试
    Fault,          // 故障
    Maintenance     // 维护
};

// 时间源
enum class TimeSource {
    NTP,            // NTP
    Beidou,         // 北斗
    GPS,            // GPS
    Local,          // 本机
    Manual          // 手动
};

// 波束 ID
enum class BeamId {
    LOS1 = 0,       // 0°
    LOS2 = 1,       // 72°
    LOS3 = 2,       // 144°
    LOS4 = 3,       // 216°
    LOS5 = 4        // 288°
};

// 波束状态
enum class BeamStatus {
    Normal,         // 正常
    WeakSignal,     // 弱信号
    Occluded,       // 遮挡
    PhaseError,     // 相位异常
    CommError,      // 通信异常
    Interpolated,   // 插值补偿
    Disabled        // 禁用
};

// 告警严重级别
enum class AlarmSeverity {
    Info,           // 提示
    Warning,        // 一般
    Important,      // 重要
    Critical        // 严重
};

// 告警来源
enum class AlarmSource {
    Device,         // 设备
    Beam,           // 波束
    Algorithm,      // 算法
    Communication,  // 通信
    Storage,        // 存储
    Config,         // 配置
    Environment     // 环境
};

// 数据状态标志
enum class StatusCode {
    Valid,              // 有效
    Invalid,            // 无效
    Interpolated,       // 插值
    LowCNR,             // 低 CNR
    Occluded,           // 遮挡
    OutOfPhysicalRange, // 超出物理范围
    InsufficientBeams,  // 波束不足
    LowConfidence       // 低置信度
};

// 时间质量
enum class TimeQuality {
    Synchronized,   // 同步
    Interpolated,   // 插值
    LocalTime,      // 本机时间
    TimeAnomaly     // 时间异常
};

// 帧头帧尾
constexpr uint16_t FRAME_HEADER = 0xAA55;
constexpr uint16_t FRAME_TAIL = 0x55AA;

// 最大帧长
constexpr int MAX_FRAME_SIZE = 4096;

// 命令码定义
enum class CommandCode : uint16_t {
    // 查询类 (0x0100-0x01FF)
    QueryDeviceInfo = 0x0100,
    QueryWindProfile = 0x0101,
    QueryBeamState = 0x0102,
    QueryDeviceHealth = 0x0103,
    QueryParameters = 0x0104,
    QueryAlarmHistory = 0x0105,

    // 控制类 (0x0200-0x02FF)
    ControlStartMeasure = 0x0200,
    ControlStopMeasure = 0x0201,
    ControlRestart = 0x0202,
    ControlApplyConfig = 0x0203,
    ControlCalibrate = 0x0204,

    // 数据推送类 (0x8100-0x81FF)
    PushWindProfile = 0x8100,
    PushBeamState = 0x8101,
    PushDeviceHealth = 0x8102,
    PushSpectrum = 0x8103,
    PushAlarm = 0x8104,

    // 响应类
    ResponseSuccess = 0x0000,
    ResponseError = 0x0001,
};

#endif // RADARTYPES_H
