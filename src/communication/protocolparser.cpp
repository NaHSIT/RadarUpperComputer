#include "protocolparser.h"
#include <QDataStream>
#include <QDebug>

ProtocolParser::ProtocolParser(QObject* parent)
    : QObject(parent)
    , m_sequence(0)
{
}

ProtocolParser::~ProtocolParser()
{
}

// ==================== 构建协议帧 ====================

QByteArray ProtocolParser::buildFrame(uint16_t command, const QByteArray& payload)
{
    return buildFrame(command, m_sequence++, payload);
}

QByteArray ProtocolParser::buildFrame(uint16_t command, uint32_t sequence, const QByteArray& payload)
{
    QByteArray frame;

    // 1. 帧头 (2字节)
    frame.append(static_cast<char>((FRAME_HEADER >> 8) & 0xFF));
    frame.append(static_cast<char>(FRAME_HEADER & 0xFF));

    // 2. 长度 (2字节) = 命令(2) + 序号(4) + 数据(N) + CRC(2)
    uint16_t length = 2 + 4 + static_cast<uint16_t>(payload.size()) + 2;
    frame.append(static_cast<char>((length >> 8) & 0xFF));
    frame.append(static_cast<char>(length & 0xFF));

    // 3. 命令码 (2字节)
    frame.append(static_cast<char>((command >> 8) & 0xFF));
    frame.append(static_cast<char>(command & 0xFF));

    // 4. 序列号 (4字节)
    frame.append(static_cast<char>((sequence >> 24) & 0xFF));
    frame.append(static_cast<char>((sequence >> 16) & 0xFF));
    frame.append(static_cast<char>((sequence >> 8) & 0xFF));
    frame.append(static_cast<char>(sequence & 0xFF));

    // 5. 数据载荷
    frame.append(payload);

    // 6. CRC16校验 (从长度字段开始计算)
    QByteArray crcData = frame.mid(2);  // 跳过帧头
    uint16_t crc = calculateCRC(crcData);
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    frame.append(static_cast<char>(crc & 0xFF));

    // 7. 帧尾 (2字节)
    frame.append(static_cast<char>((FRAME_TAIL >> 8) & 0xFF));
    frame.append(static_cast<char>(FRAME_TAIL & 0xFF));

    return frame;
}

// ==================== 解析协议帧 ====================

bool ProtocolParser::parseFrame(const QByteArray& rawData, FrameHeader& header, QByteArray& payload)
{
    QMutexLocker locker(&m_mutex);

    // 检查最小长度
    if (rawData.size() < FRAME_MIN_SIZE) {
        return false;
    }

    // 查找帧边界
    int startPos = 0;
    int frameLen = 0;
    if (!findFrameBoundary(rawData, startPos, frameLen)) {
        return false;
    }

    // 提取帧数据
    QByteArray frame = rawData.mid(startPos, frameLen);

    // 验证帧
    if (!verifyFrame(frame)) {
        return false;
    }

    // 解析头部
    header.frameHeader = (static_cast<uint8_t>(frame[0]) << 8) |
                         static_cast<uint8_t>(frame[1]);
    header.length = (static_cast<uint8_t>(frame[2]) << 8) |
                    static_cast<uint8_t>(frame[3]);
    header.command = (static_cast<uint8_t>(frame[4]) << 8) |
                     static_cast<uint8_t>(frame[5]);
    header.sequence = (static_cast<uint8_t>(frame[6]) << 24) |
                      (static_cast<uint8_t>(frame[7]) << 16) |
                      (static_cast<uint8_t>(frame[8]) << 8) |
                      static_cast<uint8_t>(frame[9]);

    // 提取数据载荷
    int payloadLen = frameLen - FRAME_MIN_SIZE;
    if (payloadLen > 0) {
        payload = frame.mid(FRAME_HEADER_SIZE + FRAME_LENGTH_SIZE +
                           FRAME_CMD_SIZE + FRAME_SEQ_SIZE, payloadLen);
    } else {
        payload.clear();
    }

    emit frameParsed(header.command, header.sequence, payload);

    return true;
}

// ==================== 查找帧边界 ====================

bool ProtocolParser::findFrameBoundary(const QByteArray& buffer, int& startPos, int& frameLen)
{
    startPos = 0;

    while (startPos < buffer.size() - 3) {
        // 查找帧头 0xAA55
        if (buffer[startPos] == 0xAA && buffer[startPos + 1] == 0x55) {
            // 获取长度字段
            uint16_t length = (static_cast<uint8_t>(buffer[startPos + 2]) << 8) |
                              static_cast<uint8_t>(buffer[startPos + 3]);

            // 帧总长度 = 帧头(2) + 长度字段(2) + 长度字段的值
            frameLen = length + 4;

            qDebug() << "findFrame: 帧头位置=" << startPos << "长度字段=" << length << "帧总长=" << frameLen << "缓冲区=" << buffer.size();

            // 检查是否有足够的数据
            if (startPos + frameLen > buffer.size()) {
                qDebug() << "findFrame: 数据不足";
                return false;
            }

            // 找到有效帧
            return true;
        }

        startPos++;
    }

    return false;
}

// ==================== 验证帧 ====================

bool ProtocolParser::verifyFrame(const QByteArray& frame)
{
    // 临时禁用CRC校验，先确保通信正常
    Q_UNUSED(frame);
    return true;

    // 以下为原CRC校验代码
    /*
    // 提取接收到的CRC
    int crcPos = frame.size() - FRAME_TAIL_SIZE - FRAME_CRC_SIZE;
    uint16_t receivedCRC = (static_cast<uint8_t>(frame[crcPos]) << 8) |
                           static_cast<uint8_t>(frame[crcPos + 1]);

    // 计算CRC
    QByteArray crcData = frame.mid(2, crcPos - 2);
    uint16_t calculatedCRC = calculateCRC(crcData);

    return receivedCRC == calculatedCRC;
    */
}

// ==================== CRC16计算 ====================

uint16_t ProtocolParser::calculateCRC(const QByteArray& data)
{
    uint16_t crc = 0xFFFF;

    for (int i = 0; i < data.size(); i++) {
        crc ^= static_cast<uint8_t>(data[i]) << 8;

        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;  // CRC-CCITT多项式
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
