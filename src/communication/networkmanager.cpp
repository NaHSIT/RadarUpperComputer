#include "networkmanager.h"
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QNetworkProxy>

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent)
    , m_tcpSocket(new QTcpSocket(this))
    , m_udpSocket(new QUdpSocket(this))
    , m_networkType(TCP)
    , m_connectionState(DISCONNECTED)
    , m_targetPort(0)
    , m_parser(new ProtocolParser(this))
    , m_reconnectTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_statisticsTimer(new QTimer(this))
    , m_reconnectInterval(5000)
    , m_heartbeatInterval(3000)
    , m_socketBufferSize(1024 * 1024)  // 1MB
    , m_bytesSent(0)
    , m_bytesReceived(0)
    , m_lastBytesSent(0)
    , m_lastBytesReceived(0)
    , m_recording(false)
    , m_recordFile(nullptr)
    , m_recordStream(nullptr)
{
    // 禁用代理（解决 "proxy type is invalid" 错误）
    m_tcpSocket->setProxy(QNetworkProxy::NoProxy);
    m_udpSocket->setProxy(QNetworkProxy::NoProxy);

    setupTcpConnections();
    setupUdpSocket();

    // 连接定时器
    connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkManager::onReconnectTimer);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &NetworkManager::onHeartbeatTimer);
    connect(m_statisticsTimer, &QTimer::timeout, this, &NetworkManager::onStatisticsTimer);

    // 连接解析器信号
    connect(m_parser, &ProtocolParser::frameParsed, this, &NetworkManager::frameReceived);
}

NetworkManager::~NetworkManager()
{
    disconnectFromHost();
    stopRecording();
}

// ==================== TCP连接设置 ====================

void NetworkManager::setupTcpConnections()
{
    connect(m_tcpSocket, &QTcpSocket::connected, this, &NetworkManager::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &NetworkManager::onTcpDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &NetworkManager::onTcpReadyRead);
    connect(m_tcpSocket, &QAbstractSocket::errorOccurred, this, &NetworkManager::onTcpError);

    // 设置套接字缓冲区大小
    m_tcpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, m_socketBufferSize);
    m_tcpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, m_socketBufferSize);
}

// ==================== UDP设置 ====================

void NetworkManager::setupUdpSocket()
{
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::onUdpReadyRead);
}

// ==================== 连接控制 ====================

bool NetworkManager::connectToHost(const QString& ip, int port, NetworkType type)
{
    if (m_connectionState == CONNECTED) {
        disconnectFromHost();
    }

    m_targetIp = ip;
    m_targetPort = port;
    m_networkType = type;

    m_connectionState = CONNECTING;
    emit connectionStateChanged(m_connectionState);

    if (type == TCP) {
        m_tcpSocket->connectToHost(ip, port);
    } else {
        // UDP不需要显式连接
        m_connectionState = CONNECTED;
        emit connected();
        emit connectionStateChanged(m_connectionState);
        startTimers();
    }

    return true;
}

void NetworkManager::disconnectFromHost()
{
    stopTimers();

    if (m_tcpSocket->isOpen()) {
        m_tcpSocket->close();
    }

    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }

    m_connectionState = DISCONNECTED;
    emit disconnected();
    emit connectionStateChanged(m_connectionState);
}

// ==================== 数据发送 ====================

bool NetworkManager::sendRawData(const QByteArray& data)
{
    if (m_connectionState != CONNECTED) {
        return false;
    }

    qint64 written = 0;

    if (m_networkType == TCP) {
        written = m_tcpSocket->write(data);
        m_tcpSocket->flush();
    } else {
        written = m_udpSocket->writeDatagram(data, QHostAddress(m_targetIp), m_targetPort);
    }

    if (written > 0) {
        m_bytesSent += written;

        if (m_recording) {
            writeRecordingData(data, true);
        }

        return true;
    }

    return false;
}

bool NetworkManager::sendFrame(uint16_t command, const QByteArray& payload)
{
    QByteArray frame = m_parser->buildFrame(command, payload);
    return sendRawData(frame);
}

// ==================== 快捷命令 ====================

bool NetworkManager::sendHeartbeat()
{
    return sendFrame(static_cast<uint16_t>(CommandCode::CMD_HEARTBEAT));
}

bool NetworkManager::sendStatusQuery()
{
    return sendFrame(static_cast<uint16_t>(CommandCode::CMD_STATUS_QUERY));
}

bool NetworkManager::sendStartMeasure()
{
    return sendFrame(static_cast<uint16_t>(CommandCode::CMD_START_MEASURE));
}

bool NetworkManager::sendStopMeasure()
{
    return sendFrame(static_cast<uint16_t>(CommandCode::CMD_STOP_MEASURE));
}

bool NetworkManager::sendSwitchBeam(uint8_t beamIndex)
{
    QByteArray payload;
    payload.append(static_cast<char>(beamIndex));
    return sendFrame(static_cast<uint16_t>(CommandCode::CMD_SWITCH_BEAM), payload);
}

bool NetworkManager::sendVersionQuery()
{
    return sendFrame(static_cast<uint16_t>(CommandCode::CMD_VERSION_QUERY));
}

// ==================== 性能监控 ====================

double NetworkManager::getSendRate() const
{
    qint64 elapsed = m_elapsedTimer.elapsed();
    if (elapsed <= 0) return 0.0;
    return (m_bytesSent - m_lastBytesSent) * 1000.0 / elapsed;
}

double NetworkManager::getRecvRate() const
{
    qint64 elapsed = m_elapsedTimer.elapsed();
    if (elapsed <= 0) return 0.0;
    return (m_bytesReceived - m_lastBytesReceived) * 1000.0 / elapsed;
}

void NetworkManager::resetStatistics()
{
    m_bytesSent = 0;
    m_bytesReceived = 0;
    m_lastBytesSent = 0;
    m_lastBytesReceived = 0;
    m_elapsedTimer.restart();
}

// ==================== 数据记录 ====================

bool NetworkManager::startRecording(const QString& filename)
{
    if (m_recording) {
        stopRecording();
    }

    m_recordFile = new QFile(filename);
    if (!m_recordFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        delete m_recordFile;
        m_recordFile = nullptr;
        return false;
    }

    m_recordStream = new QTextStream(m_recordFile);

    // 写入文件头
    *m_recordStream << "# 雷达数据记录文件" << Qt::endl;
    *m_recordStream << "# 开始时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << Qt::endl;
    *m_recordStream << "# 格式: [时间] [方向] [数据]" << Qt::endl;
    *m_recordStream << "# 方向: SEND/RECV" << Qt::endl;
    *m_recordStream << "---" << Qt::endl;

    m_recording = true;
    return true;
}

void NetworkManager::stopRecording()
{
    if (m_recordStream) {
        *m_recordStream << "---" << Qt::endl;
        *m_recordStream << "# 结束时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << Qt::endl;

        delete m_recordStream;
        m_recordStream = nullptr;
    }

    if (m_recordFile) {
        m_recordFile->close();
        delete m_recordFile;
        m_recordFile = nullptr;
    }

    m_recording = false;
}

void NetworkManager::writeRecordingData(const QByteArray& data, bool isSend)
{
    if (!m_recordStream) return;

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString direction = isSend ? "SEND" : "RECV";
    QString hexData = data.toHex().toUpper();

    *m_recordStream << timestamp << " " << direction << " " << hexData << Qt::endl;
}

// ==================== 配置 ====================

void NetworkManager::setReconnectInterval(int ms)
{
    m_reconnectInterval = ms;
}

void NetworkManager::setHeartbeatInterval(int ms)
{
    m_heartbeatInterval = ms;
}

void NetworkManager::setSocketBufferSize(int bytes)
{
    m_socketBufferSize = bytes;
}

// ==================== TCP槽函数 ====================

void NetworkManager::onTcpConnected()
{
    m_connectionState = CONNECTED;
    resetStatistics();
    startTimers();

    emit connected();
    emit connectionStateChanged(m_connectionState);

    qDebug() << "TCP connected to" << m_targetIp << ":" << m_targetPort;
}

void NetworkManager::onTcpDisconnected()
{
    stopTimers();
    m_connectionState = DISCONNECTED;

    emit disconnected();
    emit connectionStateChanged(m_connectionState);

    qDebug() << "TCP disconnected";
}

void NetworkManager::onTcpReadyRead()
{
    QByteArray data = m_tcpSocket->readAll();
    m_bytesReceived += data.size();

    qDebug() << "收到数据:" << data.size() << "bytes, hex:" << data.left(20).toHex();

    if (m_recording) {
        writeRecordingData(data, false);
    }

    // 添加到缓冲区
    QMutexLocker locker(&m_bufferMutex);
    m_recvBuffer.append(data);

    emit dataReceived(data);

    // 解析帧
    FrameHeader header;
    QByteArray payload;

    while (m_parser->parseFrame(m_recvBuffer, header, payload)) {
        int consumed = header.length + FRAME_HEADER_SIZE + FRAME_TAIL_SIZE;
        qDebug() << "解析帧: 命令=" << Qt::hex << header.command << "长度=" << consumed;
        m_recvBuffer.remove(0, consumed);

        // 发送帧接收信号
        emit frameReceived(header.command, header.sequence, payload);
    }
}

void NetworkManager::onTcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)

    QString errorMsg = m_tcpSocket->errorString();
    m_connectionState = ERROR_STATE;

    emit connectionError(errorMsg);
    emit connectionStateChanged(m_connectionState);

    qDebug() << "TCP error:" << errorMsg;
}

// ==================== UDP槽函数 ====================

void NetworkManager::onUdpReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        m_bytesReceived += datagram.size();

        if (m_recording) {
            writeRecordingData(datagram, false);
        }

        emit dataReceived(datagram);

        // 解析帧
        FrameHeader header;
        QByteArray payload;

        QMutexLocker locker(&m_bufferMutex);
        m_recvBuffer.append(datagram);

        while (m_parser->parseFrame(m_recvBuffer, header, payload)) {
            int consumed = header.length + FRAME_HEADER_SIZE + FRAME_TAIL_SIZE;
            m_recvBuffer.remove(0, consumed);
        }
    }
}

// ==================== 定时器槽函数 ====================

void NetworkManager::onReconnectTimer()
{
    if (m_connectionState != CONNECTED) {
        qDebug() << "Attempting to reconnect...";
        m_tcpSocket->connectToHost(m_targetIp, m_targetPort);
    }
}

void NetworkManager::onHeartbeatTimer()
{
    if (m_connectionState == CONNECTED) {
        if (!sendHeartbeat()) {
            emit heartbeatTimeout();
        }
    }
}

void NetworkManager::onStatisticsTimer()
{
    updateStatistics();
    emit statisticsUpdated(getSendRate(), getRecvRate());
}

// ==================== 私有函数 ====================

void NetworkManager::startTimers()
{
    m_reconnectTimer->start(m_reconnectInterval);
    m_heartbeatTimer->start(m_heartbeatInterval);
    m_statisticsTimer->start(1000);  // 每秒更新统计
    m_elapsedTimer.start();
}

void NetworkManager::stopTimers()
{
    m_reconnectTimer->stop();
    m_heartbeatTimer->stop();
    m_statisticsTimer->stop();
}

void NetworkManager::updateStatistics()
{
    m_lastBytesSent = m_bytesSent;
    m_lastBytesReceived = m_bytesReceived;
}
