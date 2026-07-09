#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include "FrameTypes.h"

/**
 * @brief 帧解析器
 *
 * 负责 AA55/55AA 帧的解析和构建
 */
class FrameParser : public QObject
{
    Q_OBJECT

public:
    explicit FrameParser(QObject *parent = nullptr);
    ~FrameParser() override;

    // 解析数据
    QList<Frame> parse(const QByteArray &data);

    // 构建帧
    QByteArray buildFrame(CommandCode command, uint32_t sequence, const QByteArray &payload);

    // 清空缓冲区
    void clearBuffer();

signals:
    void frameParsed(const Frame &frame);
    void errorOccurred(const QString &error);

private:
    // 查找帧头
    int findHeader(const QByteArray &buffer) const;

    // 验证帧
    bool validateFrame(const QByteArray &frame) const;

    // 计算 CRC16
    uint16_t calculateCRC16(const QByteArray &data) const;

    // 缓冲区
    QByteArray m_buffer;
};

#endif // FRAMEPARSER_H
