#ifndef TCPDATASOURCE_H
#define TCPDATASOURCE_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "domain/RadarTypes.h"

/**
 * @brief TCP 数据源
 *
 * 负责 TCP 连接和数据收发
 */
class TcpDataSource : public QObject
{
    Q_OBJECT

public:
    explicit TcpDataSource(QObject *parent = nullptr);
    ~TcpDataSource() override;

    // 连接管理
    bool connectToHost(const QString &host, quint16 port);
    void disconnectFromHost();
    bool isConnected() const;
    ConnectionState state() const { return m_state; }

    // 数据发送
    bool sendBytes(const QByteArray &data);

    // 配置
    void setReconnectEnabled(bool enabled);
    void setReconnectInterval(int ms);
    void setTimeout(int ms);

signals:
    void bytesReceived(const QByteArray &data);
    void stateChanged(ConnectionState state);
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();

private:
    void setState(ConnectionState state);

    QTcpSocket *m_socket;
    QTimer *m_reconnectTimer;
    ConnectionState m_state;
    bool m_reconnectEnabled;
    int m_reconnectInterval;
    int m_timeout;
    QString m_host;
    quint16 m_port;
};

#endif // TCPDATASOURCE_H
