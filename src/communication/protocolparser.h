#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <functional>
#include "frame.h"

class ProtocolParser : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolParser(QObject* parent = nullptr);
    ~ProtocolParser();

    // ==================== 发送接口 ====================

    /**
     * @brief 构建协议帧
     * @param command 命令码
     * @param payload 数据载荷
     * @return 完整的协议帧
     */
    QByteArray buildFrame(uint16_t command, const QByteArray& payload = QByteArray());

    /**
     * @brief 构建带序列号的协议帧
     * @param command 命令码
     * @param sequence 序列号
     * @param payload 数据载荷
     * @return 完整的协议帧
     */
    QByteArray buildFrame(uint16_t command, uint32_t sequence, const QByteArray& payload = QByteArray());

    // ==================== 接收解析 ====================

    /**
     * @brief 解析接收到的数据
     * @param rawData 原始数据
     * @param header 输出的帧头信息
     * @param payload 输出的数据载荷
     * @return 解析是否成功
     */
    bool parseFrame(const QByteArray& rawData, FrameHeader& header, QByteArray& payload);

    // ==================== CRC计算 ====================

    /**
     * @brief 计算CRC16校验
     * @param data 数据
     * @return CRC16值
     */
    static uint16_t calculateCRC(const QByteArray& data);

    // ==================== 工具函数 ====================

    /**
     * @brief 获取当前序列号
     */
    uint32_t getCurrentSequence() const { return m_sequence; }

    /**
     * @brief 重置序列号
     */
    void resetSequence() { m_sequence = 0; }

signals:
    // ==================== 信号 ====================

    /**
     * @brief 解析成功信号
     */
    void frameParsed(uint16_t command, uint32_t sequence, const QByteArray& payload);

    /**
     * @brief 解析错误信号
     */
    void parseError(const QString& error);

private:
    // ==================== 私有函数 ====================

    /**
     * @brief 查找帧边界
     */
    bool findFrameBoundary(const QByteArray& buffer, int& startPos, int& frameLen);

    /**
     * @brief 验证帧完整性
     */
    bool verifyFrame(const QByteArray& frame);

    // ==================== 私有变量 ====================

    QMutex m_mutex;          // 互斥锁
    uint32_t m_sequence;     // 发送序列号
};

#endif // PROTOCOLPARSER_H
