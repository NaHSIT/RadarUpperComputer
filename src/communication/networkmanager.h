#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QByteArray>
#include <QMutex>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include "frame.h"
#include "protocolparser.h"

/**
 * @brief 网络管理器
 * 支持TCP和UDP通信，用于雷达数据传输
 */
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    // 网络类型
    enum NetworkType {
        TCP,    // TCP连接
        UDP     // UDP连接
    };

    // 连接状态
    enum ConnectionState {
        DISCONNECTED,   // 未连接
        CONNECTING,     // 连接中
        CONNECTED,      // 已连接
        ERROR_STATE     // 错误状态
    };

    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();

    // ==================== 连接控制 ====================

    /**
     * @brief 连接到雷达设备
     * @param ip IP地址
     * @param port 端口号
     * @param type 网络类型 (TCP/UDP)
     * @return 连接是否成功
     */
    bool connectToHost(const QString& ip, int port, NetworkType type = TCP);

    /**
     * @brief 断开连接
     */
    void disconnectFromHost();

    /**
     * @brief 获取连接状态
     */
    ConnectionState getConnectionState() const { return m_connectionState; }

    /**
     * @brief 是否已连接
     */
    bool isConnected() const { return m_connectionState == CONNECTED; }

    // ==================== 数据发送 ====================

    /**
     * @brief 发送原始数据
     * @param data 数据
     * @return 发送是否成功
     */
    bool sendRawData(const QByteArray& data);

    /**
     * @brief 发送协议帧
     * @param command 命令码
     * @param payload 数据载荷
     * @return 发送是否成功
     */
    bool sendFrame(uint16_t command, const QByteArray& payload = QByteArray());

    // ==================== 快捷命令 ====================

    bool sendHeartbeat();           // 发送心跳
    bool sendStatusQuery();         // 查询状态
    bool sendStartMeasure();        // 启动测量
    bool sendStopMeasure();         // 停止测量
    bool sendSwitchBeam(uint8_t beamIndex);  // 切换波束
    bool sendVersionQuery();        // 查询版本

    // ==================== 性能监控 ====================

    /**
     * @brief 获取发送字节数
     */
    uint64_t getBytesSent() const { return m_bytesSent; }

    /**
     * @brief 获取接收字节数
     */
    uint64_t getBytesReceived() const { return m_bytesReceived; }

    /**
     * @brief 获取发送速率 (字节/秒)
     */
    double getSendRate() const;

    /**
     * @brief 获取接收速率 (字节/秒)
     */
    double getRecvRate() const;

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    // ==================== 数据记录 ====================

    /**
     * @brief 开始记录数据
     * @param filename 文件名
     */
    bool startRecording(const QString& filename);

    /**
     * @brief 停止记录数据
     */
    void stopRecording();

    /**
     * @brief 是否正在记录
     */
    bool isRecording() const { return m_recording; }

    // ==================== 配置 ====================

    void setReconnectInterval(int ms);      // 设置重连间隔
    void setHeartbeatInterval(int ms);      // 设置心跳间隔
    void setSocketBufferSize(int bytes);    // 设置套接字缓冲区大小

signals:
    // ==================== 信号 ====================

    void connected();                       // 已连接
    void disconnected();                    // 已断开
    void connectionStateChanged(ConnectionState state);  // 状态变化
    void connectionError(const QString& error);          // 连接错误

    void dataReceived(const QByteArray& data);           // 原始数据接收
    void frameReceived(uint16_t command, uint32_t sequence, const QByteArray& payload);  // 帧接收

    void heartbeatTimeout();                // 心跳超时
    void statisticsUpdated(double sendRate, double recvRate);  // 统计更新

public slots:
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpReadyRead();
    void onTcpError(QAbstractSocket::SocketError error);

    void onUdpReadyRead();

    void onReconnectTimer();
    void onHeartbeatTimer();
    void onStatisticsTimer();

private:
    void setupTcpConnections();
    void setupUdpSocket();
    void startTimers();
    void stopTimers();
    void updateStatistics();
    void writeRecordingData(const QByteArray& data, bool isSend);

    // ==================== 成员变量 ====================

    // 网络
    QTcpSocket* m_tcpSocket;
    QUdpSocket* m_udpSocket;
    NetworkType m_networkType;
    ConnectionState m_connectionState;

    // 目标地址
    QString m_targetIp;
    int m_targetPort;

    // 协议
    ProtocolParser* m_parser;

    // 定时器
    QTimer* m_reconnectTimer;
    QTimer* m_heartbeatTimer;
    QTimer* m_statisticsTimer;

    // 配置
    int m_reconnectInterval;
    int m_heartbeatInterval;
    int m_socketBufferSize;

    // 统计
    uint64_t m_bytesSent;
    uint64_t m_bytesReceived;
    QElapsedTimer m_elapsedTimer;
    uint64_t m_lastBytesSent;
    uint64_t m_lastBytesReceived;

    // 数据记录
    bool m_recording;
    QFile* m_recordFile;
    QTextStream* m_recordStream;

    // 接收缓冲区
    QByteArray m_recvBuffer;
    QMutex m_bufferMutex;
};

#endif // NETWORKMANAGER_H
