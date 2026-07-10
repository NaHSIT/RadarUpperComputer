#ifndef RADARTYPES_H
#define RADARTYPES_H

#include <cstdint>
#include <QString>
#include <QDateTime>
#include <QVector>

enum class ConnectionState {
    Offline,
    Connecting,
    Online,
    DataTimeout,
    ProtocolError
};

enum class WorkMode {
    Standby,
    Initializing,
    Running,
    Measuring,
    Debug,
    Fault,
    Maintenance
};

enum class TimeSource {
    NTP,
    Beidou,
    GPS,
    Local,
    Manual
};

enum class BeamId {
    LOS1 = 0,
    LOS2 = 1,
    LOS3 = 2,
    LOS4 = 3,
    LOS5 = 4
};

enum class BeamStatus {
    Normal,
    WeakSignal,
    Occluded,
    PhaseError,
    CommError,
    Interpolated,
    Disabled
};

enum class AlarmSeverity {
    Info,
    Warning,
    Important,
    Critical
};

enum class AlarmSource {
    Device,
    Beam,
    Algorithm,
    Communication,
    Storage,
    Config,
    Environment
};

enum class StatusCode {
    Valid,
    Invalid,
    Interpolated,
    LowCNR,
    Occluded,
    OutOfPhysicalRange,
    InsufficientBeams,
    LowConfidence
};

enum class TimeQuality {
    Synchronized,
    Interpolated,
    LocalTime,
    TimeAnomaly
};

constexpr uint16_t FRAME_HEADER = 0xAA55;
constexpr uint16_t FRAME_TAIL = 0x55AA;
constexpr int MAX_FRAME_SIZE = 4096;

enum class CommandCode : uint16_t {
    QueryDeviceInfo = 0x0100,
    QueryWindProfile = 0x0101,
    QueryBeamState = 0x0102,
    QueryDeviceHealth = 0x0103,
    QueryParameters = 0x0104,
    QueryAlarmHistory = 0x0105,

    ControlStartMeasure = 0x0200,
    ControlStopMeasure = 0x0201,
    ControlRestart = 0x0202,
    ControlApplyConfig = 0x0203,
    ControlCalibrate = 0x0204,

    PushWindProfile = 0x8100,
    PushBeamState = 0x8101,
    PushDeviceHealth = 0x8102,
    PushSpectrum = 0x8103,
    PushAlarm = 0x8104,

    ResponseSuccess = 0x0000,
    ResponseError = 0x0001
};

#endif // RADARTYPES_H
