#ifndef FRAMETYPES_H
#define FRAMETYPES_H

#include <cstdint>
#include <QByteArray>
#include "domain/RadarTypes.h"

/**
 * @brief 帧类型定义
 */

// 帧结构
#pragma pack(push, 1)
struct FrameHeader {
    uint16_t header;      // 帧头 0xAA55
    uint16_t length;      // 长度（命令+序号+负载+CRC）
    uint16_t command;     // 命令码
    uint32_t sequence;    // 序号
};

struct FrameTail {
    uint16_t crc16;       // CRC16 校验
    uint16_t tail;        // 帧尾 0x55AA
};
#pragma pack(pop)

// 完整帧结构
struct Frame {
    uint16_t header;
    uint16_t length;
    uint16_t command;
    uint32_t sequence;
    QByteArray payload;
    uint16_t crc16;
    uint16_t tail;

    // 便捷方法
    CommandCode commandCode() const { return static_cast<CommandCode>(command); }
    bool isValid() const { return header == FRAME_HEADER && tail == FRAME_TAIL; }
};

#endif // FRAMETYPES_H
