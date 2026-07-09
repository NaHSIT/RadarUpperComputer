#ifndef WEBSOCKETMANAGER_H
#define WEBSOCKETMANAGER_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#ifdef HAS_WEBSOCKETS
#include <QWebSocket>
#endif

/**
 * @brief WebSocket 管理器
 *
 * 负责 WebSocket 连接和数据收发
 */
class WebSocketManager : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketManager(QObject *parent = nullptr);
    ~WebSocketManager() override;

    // 连接管理
    bool connectToServer(const QUrl &url, const QString &token = QString());
    void disconnectFromServer();
    bool isConnected() const;

    // 消息发送
    void sendMessage(const QString &event, const QJsonObject &data);
    void subscribe(const QString &event);
    void unsubscribe(const QString &event);

    // 配置
    void setReconnectEnabled(bool enabled);
    void setReconnectInterval(int ms);
    void setHeartbeatInterval(int ms);

signals:
    void connected();
    void disconnected();
    void eventReceived(const QString &event, const QJsonObject &data);
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();
    void onHeartbeatTimer();

private:
    void processMessage(const QString &message);
    void sendHeartbeat();

#ifdef HAS_WEBSOCKETS
    QWebSocket *m_socket;
#endif
    QTimer *m_reconnectTimer;
    QTimer *m_heartbeatTimer;
    bool m_reconnectEnabled;
    int m_reconnectInterval;
    int m_heartbeatInterval;
    QUrl m_serverUrl;
    QString m_token;
    QStringList m_subscriptions;
};

#endif // WEBSOCKETMANAGER_H
