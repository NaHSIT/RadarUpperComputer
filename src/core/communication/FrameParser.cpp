#include "FrameParser.h"
#include <cstring>

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
        if (m_buffer.size() < static_cast<int>(sizeof(FrameHeader) + sizeof(FrameTail))) {
            break;  // 数据不完整
        }

        // 读取长度字段
        uint16_t length = *reinterpret_cast<const uint16_t*>(m_buffer.constData() + 2);
        int totalFrameSize = sizeof(FrameHeader) + length + sizeof(FrameTail);

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
        const FrameHeader* header = reinterpret_cast<const FrameHeader*>(frameData.constData());
        frame.header = header->header;
        frame.length = header->length;
        frame.command = header->command;
        frame.sequence = header->sequence;
        frame.payload = frameData.mid(sizeof(FrameHeader), length - sizeof(uint16_t));
        frame.crc16 = *reinterpret_cast<const uint16_t*>(frameData.constData() + totalFrameSize - sizeof(FrameTail));
        frame.tail = *reinterpret_cast<const uint16_t*>(frameData.constData() + totalFrameSize - 2);

        frames.append(frame);
        emit frameParsed(frame);
    }

    return frames;
}

QByteArray FrameParser::buildFrame(CommandCode command, uint32_t sequence, const QByteArray &payload)
{
    // 构建帧头
    QByteArray frame;
    frame.append(reinterpret_cast<const char*>(&FRAME_HEADER), 2);

    uint16_t length = sizeof(uint16_t) + sizeof(uint32_t) + payload.size() + sizeof(uint16_t);
    frame.append(reinterpret_cast<const char*>(&length), 2);

    uint16_t cmd = static_cast<uint16_t>(command);
    frame.append(reinterpret_cast<const char*>(&cmd), 2);
    frame.append(reinterpret_cast<const char*>(&sequence), 4);

    // 添加 Payload
    frame.append(payload);

    // 计算并添加 CRC16
    uint16_t crc = calculateCRC16(frame);
    frame.append(reinterpret_cast<const char*>(&crc), 2);

    // 添加帧尾
    frame.append(reinterpret_cast<const char*>(&FRAME_TAIL), 2);

    return frame;
}

void FrameParser::clearBuffer()
{
    m_buffer.clear();
}

int FrameParser::findHeader(const QByteArray &buffer) const
{
    for (int i = 0; i <= buffer.size() - 2; ++i) {
        uint16_t value = *reinterpret_cast<const uint16_t*>(buffer.constData() + i);
        if (value == FRAME_HEADER) {
            return i;
        }
    }
    return -1;
}

bool FrameParser::validateFrame(const QByteArray &frame) const
{
    if (frame.size() < static_cast<int>(sizeof(FrameHeader) + sizeof(FrameTail))) {
        return false;
    }

    // 提取 CRC
    uint16_t storedCRC = *reinterpret_cast<const uint16_t*>(frame.constData() + frame.size() - sizeof(FrameTail));

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
