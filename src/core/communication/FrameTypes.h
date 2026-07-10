#ifndef FRAMETYPES_H
#define FRAMETYPES_H

#include <cstdint>
#include <QByteArray>

#include "domain/RadarTypes.h"

#pragma pack(push, 1)
struct FrameHeader {
    uint16_t header;
    uint16_t length;
    uint16_t command;
    uint32_t sequence;
};

struct FrameTail {
    uint16_t crc16;
    uint16_t tail;
};
#pragma pack(pop)

struct Frame {
    uint16_t header;
    uint16_t length;
    uint16_t command;
    uint32_t sequence;
    QByteArray payload;
    uint16_t crc16;
    uint16_t tail;

    CommandCode commandCode() const { return static_cast<CommandCode>(command); }
    bool isValid() const { return header == FRAME_HEADER && tail == FRAME_TAIL; }
};

#endif // FRAMETYPES_H
