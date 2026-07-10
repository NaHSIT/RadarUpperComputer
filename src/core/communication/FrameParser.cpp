#include "FrameParser.h"
#include <cstring>

namespace {

uint16_t readU16BE(const char *data)
{
    const auto *bytes = reinterpret_cast<const unsigned char *>(data);
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8) | bytes[1]);
}

uint32_t readU32BE(const char *data)
{
    const auto *bytes = reinterpret_cast<const unsigned char *>(data);
    return (static_cast<uint32_t>(bytes[0]) << 24)
         | (static_cast<uint32_t>(bytes[1]) << 16)
         | (static_cast<uint32_t>(bytes[2]) << 8)
         | bytes[3];
}

void appendU16BE(QByteArray &frame, uint16_t value)
{
    frame.append(static_cast<char>((value >> 8) & 0xFF));
    frame.append(static_cast<char>(value & 0xFF));
}

void appendU32BE(QByteArray &frame, uint32_t value)
{
    frame.append(static_cast<char>((value >> 24) & 0xFF));
    frame.append(static_cast<char>((value >> 16) & 0xFF));
    frame.append(static_cast<char>((value >> 8) & 0xFF));
    frame.append(static_cast<char>(value & 0xFF));
}

}

FrameParser::FrameParser(QObject *parent)
    : QObject(parent)
{
}

FrameParser::~FrameParser()
{
}

QList<Frame> FrameParser::parse(const QByteArray &data)
{
    QList<Frame> frames;

    // 追加到缓冲区
    m_buffer.append(data);

    // 循环查找帧
    while (true) {
        // 查找帧头
        int headerPos = findHeader(m_buffer);
        if (headerPos < 0) {
            // 没有找到帧头，保留最后 1 字节
            if (m_buffer.size() > 1) {
                m_buffer = m_buffer.right(1);
            }
            break;
        }

        // 丢弃帧头之前的数据
        if (headerPos > 0) {
            m_buffer = m_buffer.mid(headerPos);
        }

        // 检查缓冲区长度是否足够
        if (m_buffer.size() < 14) {
            break;  // 数据不完整
        }

        // 读取长度字段
        uint16_t length = readU16BE(m_buffer.constData() + 2);
        int totalFrameSize = 2 + 2 + length + 2;

        // 检查帧长是否合法
        if (totalFrameSize > MAX_FRAME_SIZE) {
            emit errorOccurred("Frame size exceeds maximum limit");
            m_buffer = m_buffer.mid(1);  // 跳过 1 字节继续查找
            continue;
        }

        // 检查缓冲区是否包含完整帧
        if (m_buffer.size() < totalFrameSize) {
            break;  // 数据不完整
        }

        // 提取帧数据
        QByteArray frameData = m_buffer.left(totalFrameSize);
        m_buffer = m_buffer.mid(totalFrameSize);

        // 验证帧
        if (!validateFrame(frameData)) {
            emit errorOccurred("CRC validation failed");
            continue;
        }

        // 解析帧
        Frame frame;
        frame.header = readU16BE(frameData.constData());
        frame.length = length;
        frame.command = readU16BE(frameData.constData() + 4);
        frame.sequence = readU32BE(frameData.constData() + 6);
        const int payloadLength = static_cast<int>(length)
            - static_cast<int>(sizeof(uint16_t))
            - static_cast<int>(sizeof(uint32_t))
            - static_cast<int>(sizeof(uint16_t));
        frame.payload = frameData.mid(sizeof(FrameHeader), payloadLength);
        frame.crc16 = readU16BE(frameData.constData() + totalFrameSize - sizeof(FrameTail));
        frame.tail = readU16BE(frameData.constData() + totalFrameSize - 2);

        frames.append(frame);
        emit frameParsed(frame);
    }

    return frames;
}

QByteArray FrameParser::buildFrame(CommandCode command, uint32_t sequence, const QByteArray &payload)
{
    // 构建帧头
    QByteArray frame;
    uint16_t length = sizeof(uint16_t) + sizeof(uint32_t) + payload.size() + sizeof(uint16_t);
    appendU16BE(frame, FRAME_HEADER);
    appendU16BE(frame, length);
    appendU16BE(frame, static_cast<uint16_t>(command));
    appendU32BE(frame, sequence);

    // 添加 Payload
    frame.append(payload);

    // 计算并添加 CRC16
    uint16_t crc = calculateCRC16(frame);
    appendU16BE(frame, crc);

    // 添加帧尾
    appendU16BE(frame, FRAME_TAIL);

    return frame;
}

void FrameParser::clearBuffer()
{
    m_buffer.clear();
}

int FrameParser::findHeader(const QByteArray &buffer) const
{
    for (int i = 0; i <= buffer.size() - 2; ++i) {
        uint16_t value = readU16BE(buffer.constData() + i);
        if (value == FRAME_HEADER) {
            return i;
        }
    }
    return -1;
}

bool FrameParser::validateFrame(const QByteArray &frame) const
{
    if (frame.size() < 14) {
        return false;
    }

    // 提取 CRC
    uint16_t storedCRC = readU16BE(frame.constData() + frame.size() - sizeof(FrameTail));

    // 计算 CRC（不包括 CRC 字段和帧尾）
    QByteArray dataForCRC = frame.left(frame.size() - sizeof(FrameTail));
    uint16_t calculatedCRC = calculateCRC16(dataForCRC);

    return storedCRC == calculatedCRC;
}

uint16_t FrameParser::calculateCRC16(const QByteArray &data) const
{
    uint16_t crc = 0xFFFF;

    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}
